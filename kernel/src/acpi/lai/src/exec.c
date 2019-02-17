
/*
 * Lux ACPI Implementation
 * Copyright (C) 2018-2019 by Omar Muhamed
 */

#include "lai.h"
#include "exec_impl.h"
#include "ns_impl.h"

static int debug_opcodes = 0;

/* ACPI Control Method Execution */
/* Type1Opcode := DefBreak | DefBreakPoint | DefContinue | DefFatal | DefIfElse |
   DefLoad | DefNoop | DefNotify | DefRelease | DefReset | DefReturn |
   DefSignal | DefSleep | DefStall | DefUnload | DefWhile */

void acpi_eval_operand(acpi_object_t *destination, acpi_state_t *state, uint8_t *code);

// Prepare the interpreter state for a control method call.
// Param: acpi_state_t *state - will store method name and arguments
// Param: acpi_nsnode_t *method - identifies the control method

void acpi_init_state(acpi_state_t *state) {
    acpi_memset(state, 0, sizeof(acpi_state_t));
    state->stack_ptr = -1;
    state->context_ptr = -1;
}

// Finalize the interpreter state. Frees all memory owned by the state.

void acpi_finalize_state(acpi_state_t *state) {
    acpi_free_object(&state->retvalue);
    for(int i = 0; i < 7; i++)
    {
        acpi_free_object(&state->arg[i]);
    }

    for(int i = 0; i < 8; i++)
    {
        acpi_free_object(&state->local[i]);
    }
}

// Pushes a new item to the opstack and returns it.
static acpi_object_t *acpi_exec_push_opstack_or_die(acpi_state_t *state) {
    if(state->opstack_ptr == 16)
        acpi_panic("operand stack overflow\n");
    acpi_object_t *object = &state->opstack[state->opstack_ptr];
    acpi_memset(object, 0, sizeof(acpi_object_t));
    state->opstack_ptr++;
    return object;
}

// Returns the n-th item from the opstack.
static acpi_object_t *acpi_exec_get_opstack(acpi_state_t *state, int n) {
    if(n >= state->opstack_ptr)
        acpi_panic("opstack access out of bounds"); // This is an internal execution error.
    return &state->opstack[n];
}

// Removes n items from the opstack.
static void acpi_exec_pop_opstack(acpi_state_t *state, int n) {
    for(int k = 0; k < n; k++)
        acpi_free_object(&state->opstack[state->opstack_ptr - k - 1]);
    state->opstack_ptr -= n;
}

// Pushes a new item to the execution stack and returns it.
static acpi_stackitem_t *acpi_exec_push_stack_or_die(acpi_state_t *state) {
    state->stack_ptr++;
    if(state->stack_ptr == 16)
        acpi_panic("execution engine stack overflow\n");
    return &state->stack[state->stack_ptr];
}

// Returns the n-th item from the top of the stack.
static acpi_stackitem_t *acpi_exec_peek_stack(acpi_state_t *state, int n) {
    if(state->stack_ptr - n < 0)
        return NULL;
    return &state->stack[state->stack_ptr - n];
}

// Returns the last item of the stack.
static acpi_stackitem_t *acpi_exec_peek_stack_back(acpi_state_t *state) {
    return acpi_exec_peek_stack(state, 0);
}

// Removes n items from the stack.
static void acpi_exec_pop_stack(acpi_state_t *state, int n) {
    state->stack_ptr -= n;
}

// Removes the last item from the stack.
static void acpi_exec_pop_stack_back(acpi_state_t *state) {
    acpi_exec_pop_stack(state, 1);
}

// Returns the acpi_stackitem_t pointed to by the state's context_ptr.
static acpi_stackitem_t *acpi_exec_context(acpi_state_t *state) {
    return &state->stack[state->context_ptr];
}

// Updates the state's context_ptr to point to the innermost context item.
static void acpi_exec_update_context(acpi_state_t *state) {
    int j = 0;
    acpi_stackitem_t *ctx_item;
    while(1) {
        ctx_item = acpi_exec_peek_stack(state, j);
        if(!ctx_item)
            break;
        if(ctx_item->kind == LAI_POPULATE_CONTEXT_STACKITEM
                || ctx_item->kind == LAI_METHOD_CONTEXT_STACKITEM)
            break;
        j++;
    }

    state->context_ptr = state->stack_ptr - j;
}

static int acpi_compare(acpi_object_t *lhs, acpi_object_t *rhs) {
    // TODO: Allow comparsions of strings and buffers as in the spec.
    if(lhs->type != ACPI_INTEGER || rhs->type != ACPI_INTEGER)
        acpi_panic("comparsion of object type %d with type %d is not implemented\n",
                lhs->type, rhs->type);
    return lhs->integer - rhs->integer;
}

static void acpi_exec_reduce(int opcode, acpi_state_t *state, acpi_object_t *operands,
        acpi_object_t *reduction_res) {
    if(debug_opcodes)
        acpi_debug("acpi_exec_reduce: opcode 0x%02X\n", opcode);
    acpi_object_t result = {0};
    switch(opcode) {
    case STORE_OP:
        acpi_load_operand(state, &operands[0], &result);
        acpi_store_operand(state, &operands[1], &result);
        break;
    case NOT_OP:
    {
        acpi_object_t operand = {0};
        acpi_load_operand(state, operands, &operand);

        result.type = ACPI_INTEGER;
        result.integer = ~operand.integer;
        acpi_store_operand(state, &operands[1], &result);
        break;
    }
    case ADD_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer + rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case SUBTRACT_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer - rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case MULTIPLY_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer * rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case AND_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer & rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case OR_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer | rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case XOR_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer ^ rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case SHL_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer << rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case SHR_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer >> rhs.integer;
        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case DIVIDE_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        acpi_object_t mod = {0};
        acpi_object_t div = {0};
        mod.type = ACPI_INTEGER;
        div.type = ACPI_INTEGER;
        mod.integer = lhs.integer % rhs.integer;
        div.integer = lhs.integer / rhs.integer;
        acpi_store_operand(state, &operands[2], &mod);
        acpi_store_operand(state, &operands[3], &div);
        break;
    }
    case INCREMENT_OP:
        acpi_load_operand(state, operands, &result);
        result.integer++;
        acpi_store_operand(state, operands, &result);
        break;
    case DECREMENT_OP:
        acpi_load_operand(state, operands, &result);
        result.integer--;
        acpi_store_operand(state, operands, &result);
        break;
    case LNOT_OP:
    {
        acpi_object_t operand = {0};
        acpi_load_operand(state, operands, &operand);

        result.type = ACPI_INTEGER;
        result.integer = !operand.integer;
        break;
    }
    case LAND_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer && rhs.integer;
        break;
    }
    case LOR_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = lhs.integer || rhs.integer;
        break;
    }
    case LEQUAL_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = !acpi_compare(&lhs, &rhs);
        break;
    }
    case LLESS_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = acpi_compare(&lhs, &rhs) < 0;
        break;
    }
    case LGREATER_OP:
    {
        acpi_object_t lhs = {0};
        acpi_object_t rhs = {0};
        acpi_load_operand(state, &operands[0], &lhs);
        acpi_load_operand(state, &operands[1], &rhs);

        result.type = ACPI_INTEGER;
        result.integer = acpi_compare(&lhs, &rhs) > 0;
        break;
    }
    case INDEX_OP:
    {
        acpi_object_t storage = {0};
        acpi_object_t index = {0};
        acpi_alias_operand(state, &operands[0], &storage);
        acpi_load_operand(state, &operands[1], &index);
        int n = index.integer;

        if(storage.type == ACPI_STRING_REFERENCE)
        {
            if(n >= acpi_strlen(storage.string))
                acpi_panic("string Index() out of bounds");
            result.type = ACPI_STRING_INDEX;
            result.string = storage.string;
            result.integer = n;
        }else if(storage.type == ACPI_BUFFER_REFERENCE)
        {
            if(n >= storage.buffer_size)
                acpi_panic("buffer Index() out of bounds");
            result.type = ACPI_BUFFER_INDEX;
            result.buffer = storage.buffer;
            result.integer = n;
        }else if(storage.type == ACPI_PACKAGE_REFERENCE)
        {
            if(n >= storage.package_size)
                acpi_panic("package Index() out of bounds");
            result.type = ACPI_PACKAGE_INDEX;
            result.package = storage.package;
            result.integer = n;
        }else
            acpi_panic("Index() is only defined for buffers, strings and packages\n");
        acpi_free_object(&storage);

        acpi_store_operand(state, &operands[2], &result);
        break;
    }
    case DEREF_OP:
    {
        acpi_object_t ref = {0};
        acpi_load_operand(state, &operands[0], &ref);

        if(ref.type == ACPI_STRING_INDEX)
        {
            result.type = ACPI_INTEGER;
            result.integer = ref.string[ref.integer];
        }else if(ref.type == ACPI_BUFFER)
        {
            uint8_t *window = ref.buffer;
            result.type = ACPI_INTEGER;
            result.integer = window[ref.integer];
        }else if(ref.type == ACPI_PACKAGE)
        {
            acpi_copy_object(&result, &ref.package[ref.integer]);
        }else
            acpi_panic("DeRefOf() is only defined for references\n");
        break;
    }
    case SIZEOF_OP:
    {
        acpi_object_t object = {0};
        acpi_load_operand(state, &operands[0], &object);

        if(object.type == ACPI_STRING)
        {
            result.type = ACPI_INTEGER;
            result.integer = acpi_strlen(object.string);
        }else if(object.type == ACPI_BUFFER)
        {
            result.type = ACPI_INTEGER;
            result.integer = object.buffer_size;
        }else if(object.type == ACPI_PACKAGE)
        {
            result.type = ACPI_INTEGER;
            result.integer = object.package_size;
        }else
            acpi_panic("SizeOf() is only defined for buffers, strings and packages\n");
        break;
    }
    case (EXTOP_PREFIX << 8) | CONDREF_OP:
    {
        acpi_object_t *operand = &operands[0];
        acpi_object_t *target = &operands[1];

        // TODO: The resolution code should be shared with REF_OP.
        acpi_object_t ref = {0};
        switch(operand->type)
        {
        case ACPI_UNRESOLVED_NAME:
        {
            acpi_nsnode_t *handle = acpi_exec_resolve(operand->name);
            if(handle) {
                ref.type = ACPI_HANDLE;
                ref.handle = handle;
            }
            break;
        }
        default:
            acpi_panic("CondRefOp() is only defined for names\n");
        }

        if(ref.type)
        {
            result.type = ACPI_INTEGER;
            result.type = 1;
            acpi_store_operand(state, target, &ref);
        }else
        {
            result.type = ACPI_INTEGER;
            result.type = 0;
        }
        break;
    }
    default:
        acpi_panic("undefined opcode in acpi_exec_reduce: %02X\n", opcode);
    }

    acpi_move_object(reduction_res, &result);
}

// acpi_exec_run(): Internal function, executes actual AML opcodes
// Param:  uint8_t *method - pointer to method opcodes
// Param:  acpi_state_t *state - machine state
// Return: int - 0 on success

static int acpi_exec_run(uint8_t *method, acpi_state_t *state)
{
    acpi_stackitem_t *item;
    while((item = acpi_exec_peek_stack_back(state)))
    {
        // Package-size encoding (and similar) needs to know the PC of the opcode.
        // If an opcode sequence contains a pkgsize, the sequence generally ends at:
        //     opcode_pc + pkgsize + opcode size.
        int opcode_pc = state->pc;

        // Whether we use the result of an expression or not.
        // If yes, it will be pushed onto the opstack after the expression is computed.
        int exec_result_mode = LAI_EXEC_MODE;

        if(item->kind == LAI_POPULATE_CONTEXT_STACKITEM)
        {
			if(state->pc == item->ctx_limit)
            {
				acpi_exec_pop_stack_back(state);
                acpi_exec_update_context(state);
				continue;
			}

            if(state->pc > item->ctx_limit) // This would be an interpreter bug.
                acpi_panic("namespace population escaped out of code range\n");
        }else if(item->kind == LAI_METHOD_CONTEXT_STACKITEM)
        {
			// ACPI does an implicit Return(0) at the end of a control method.
			if(state->pc == state->limit)
			{
				if(state->opstack_ptr) // This is an internal error.
					acpi_panic("opstack is not empty before return\n");
				acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
				result->type = ACPI_INTEGER;
				result->integer = 0;

				acpi_exec_pop_stack_back(state);
                acpi_exec_update_context(state);
				continue;
			}
        }else if(item->kind == LAI_EVALOPERAND_STACKITEM)
        {
            if(state->opstack_ptr == item->opstack_frame + 1)
            {
                acpi_exec_pop_stack_back(state);
                return 0;
            }

            exec_result_mode = LAI_OBJECT_MODE;
        }else if(item->kind == LAI_PKG_INITIALIZER_STACKITEM)
        {
            acpi_object_t *frame = acpi_exec_get_opstack(state, item->opstack_frame);
            acpi_object_t *package = &frame[0];
            acpi_object_t *initializer = &frame[1];
            if(state->opstack_ptr == item->opstack_frame + 2)
            {
                if(item->pkg_index == package->package_size)
                    acpi_panic("package initializer overflows its size");
                LAI_ENSURE(item->pkg_index < package->package_size);

                acpi_move_object(&package->package[item->pkg_index], initializer);
                item->pkg_index++;
                acpi_exec_pop_opstack(state, 1);
            }
            LAI_ENSURE(state->opstack_ptr == item->opstack_frame + 1);

            if(state->pc == item->pkg_end)
            {
                if(item->pkg_result_mode == LAI_EXEC_MODE)
                    acpi_exec_pop_opstack(state, 1);
                else
                    LAI_ENSURE(item->pkg_result_mode == LAI_DATA_MODE
                               || item->pkg_result_mode == LAI_OBJECT_MODE);

                acpi_exec_pop_stack_back(state);
                continue;
            }

            if(state->pc > item->pkg_end) // This would be an interpreter bug.
                acpi_panic("package initializer escaped out of code range\n");

            exec_result_mode = LAI_DATA_MODE;
        }else if(item->kind == LAI_OP_STACKITEM)
        {
            int k = state->opstack_ptr - item->opstack_frame;
//            acpi_debug("got %d parameters\n", k);
            if(!item->op_arg_modes[k]) {
                acpi_object_t result = {0};
                acpi_object_t *operands = acpi_exec_get_opstack(state, item->opstack_frame);
                acpi_exec_reduce(item->op_opcode, state, operands, &result);
                acpi_exec_pop_opstack(state, k);

                if(item->op_result_mode == LAI_OBJECT_MODE
                        || item->op_result_mode == LAI_TARGET_MODE)
                {
                    acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
                    acpi_move_object(opstack_res, &result);
                }else
                    LAI_ENSURE(item->op_result_mode == LAI_EXEC_MODE);

                acpi_exec_pop_stack_back(state);
                continue;
            }

            exec_result_mode = item->op_arg_modes[k];
        }else if(item->kind == LAI_LOOP_STACKITEM)
        {
            if(state->pc == item->loop_pred)
            {
                // We are at the beginning of a loop. We check the predicate; if it is false,
                // we jump to the end of the loop and remove the stack item.
                acpi_object_t predicate = {0};
                acpi_eval_operand(&predicate, state, method);
                if(!predicate.integer)
                {
                    state->pc = item->loop_end;
                    acpi_exec_pop_stack_back(state);
                }
                continue;
            }else if(state->pc == item->loop_end)
            {
                // Unconditionally jump to the loop's predicate.
                state->pc = item->loop_pred;
                continue;
            }

            if (state->pc > item->loop_end) // This would be an interpreter bug.
                acpi_panic("execution escaped out of While() body\n");
        }else if(item->kind == LAI_COND_STACKITEM)
        {
            // If the condition wasn't taken, execute the Else() block if it exists
            if(!item->cond_taken)
            {
                if(method[state->pc] == ELSE_OP)
                {
                    size_t else_size;
                    state->pc++;
                    state->pc += acpi_parse_pkgsize(method + state->pc, &else_size);
                }

                acpi_exec_pop_stack_back(state);
                continue;
            }

            // Clean up the execution stack at the end of If().
            if(state->pc == item->cond_end)
            {
                // Consume a follow-up Else() opcode.
                if(state->pc < state->limit && method[state->pc] == ELSE_OP) {
                    state->pc++;
                    size_t else_size;
                    state->pc += acpi_parse_pkgsize(method + state->pc, &else_size);
                    state->pc = opcode_pc + else_size + 1;
                }

                acpi_exec_pop_stack_back(state);
                continue;
            }
        }else
            acpi_panic("unexpected acpi_stackitem_t\n");

        if(state->pc >= state->limit) // This would be an interpreter bug.
            acpi_panic("execution escaped out of code range (PC is 0x%x with limit 0x%x)\n",
                    state->pc, state->limit);

        acpi_stackitem_t *ctx_item = acpi_exec_context(state);
        acpi_nsnode_t *ctx_handle = ctx_item->ctx_handle;

        // Process names.
        if(acpi_is_name(method[state->pc])) {
            acpi_object_t unresolved = {0};
            unresolved.type = ACPI_UNRESOLVED_NAME;
            state->pc += acpins_resolve_path(ctx_handle, unresolved.name, method + state->pc);

            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_TARGET_MODE)
            {
                if(debug_opcodes)
                    acpi_debug("parsing name %s [@ %d]\n", unresolved.name, opcode_pc);

                acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
                acpi_move_object(opstack_res, &unresolved);
            }else
            {
                LAI_ENSURE(exec_result_mode == LAI_OBJECT_MODE
                           || exec_result_mode == LAI_EXEC_MODE);
                acpi_nsnode_t *handle = acpi_exec_resolve(unresolved.name);
                if(!handle)
                    acpi_panic("undefined reference %s in object mode\n", unresolved.name);

                acpi_object_t result = {0};
                if(handle->type == ACPI_NAMESPACE_METHOD)
                {
                    if(debug_opcodes)
                        acpi_debug("parsing invocation %s [@ %d]\n", unresolved.name, opcode_pc);

                    acpi_state_t nested_state;
                    acpi_init_state(&nested_state);
                    int argc = handle->method_flags & METHOD_ARGC_MASK;
                    for(int i = 0; i < argc; i++)
                        acpi_eval_operand(&nested_state.arg[i], state, method);

                    acpi_exec_method(handle, &nested_state);
                    acpi_move_object(&result, &nested_state.retvalue);
                    acpi_finalize_state(&nested_state);
                }else
                {
                    if(debug_opcodes)
                        acpi_debug("parsing name %s [@ %d]\n", unresolved.name, opcode_pc);

                    acpi_load_ns(handle, &result);
                }

                if(exec_result_mode == LAI_OBJECT_MODE)
                {
                    acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
                    acpi_move_object(opstack_res, &result);
                }
            }
            acpi_free_object(&unresolved);
            continue;
        }

        /* General opcodes */
        int opcode;
        if(method[state->pc] == EXTOP_PREFIX)
        {
            if(state->pc + 1 == state->limit)
                acpi_panic("two-byte opcode on method boundary\n");
            opcode = (EXTOP_PREFIX << 8) | method[state->pc + 1];
        }else
            opcode = method[state->pc];
        if(debug_opcodes)
            acpi_debug("parsing opcode 0x%02x [@ %d]\n", opcode, opcode_pc);

        // This switch handles the majority of all opcodes.
        switch(opcode)
        {
        case NOP_OP:
            state->pc++;
            break;

        case ZERO_OP:
            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_INTEGER;
                result->integer = 0;
            }else if(exec_result_mode == LAI_TARGET_MODE)
            {
                // In target mode, ZERO_OP generates a null target and not an integer!
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_NULL_NAME;
            }else
            {
                acpi_warn("Zero() in execution mode has no effect\n");
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            }
            state->pc++;
            break;
        case ONE_OP:
            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_INTEGER;
                result->integer = 1;
            }else
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            state->pc++;
            break;
        case ONES_OP:
            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_INTEGER;
                result->integer = ~((uint64_t)0);
            }else
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            state->pc++;
            break;

        case BYTEPREFIX:
        case WORDPREFIX:
        case DWORDPREFIX:
        case QWORDPREFIX:
        {
            uint64_t integer;
            size_t integer_size = acpi_eval_integer(method + state->pc, &integer);
            if(!integer_size)
                acpi_panic("failed to parse integer opcode\n");
            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_INTEGER;
                result->integer = integer;
            }else
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            state->pc += integer_size;
            break;
        }
        case STRINGPREFIX:
        {
            state->pc++;

            size_t n = 0; // Determine the length of null-terminated string.
            while(state->pc + n < state->limit && method[state->pc + n])
                n++;
            if(state->pc + n == state->limit)
                acpi_panic("unterminated string in AML code");

            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
                opstack_res->type = ACPI_STRING;
                opstack_res->string = acpi_malloc(n + 1);
                acpi_memcpy(opstack_res->string, method + state->pc, n);
                opstack_res->string[n] = 0;
            }else
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            state->pc += n + 1;
            break;
        }
        case BUFFER_OP:
        {
            state->pc++;        // Skip BUFFER_OP.

            // Size of the buffer initializer.
            // Note that not all elements of the buffer need to be initialized.
            size_t encoded_size;
            state->pc += acpi_parse_pkgsize(method + state->pc, &encoded_size);

            // The size of the buffer in bytes.
            acpi_object_t buffer_size = {0};
            acpi_eval_operand(&buffer_size, state, method);

            acpi_object_t result = {0};
            result.type = ACPI_BUFFER;
            result.buffer_size = buffer_size.integer;
            result.buffer = acpi_malloc(buffer_size.integer);
            if(!result.buffer)
                acpi_panic("failed to allocate memory for AML buffer");
            acpi_memset(result.buffer, 0, buffer_size.integer);

            int initial_size = (opcode_pc + encoded_size + 1) - state->pc;
            if(initial_size < 0)
                acpi_panic("buffer initializer has negative size\n");
            if(initial_size > result.buffer_size)
                acpi_panic("buffer initializer overflows buffer\n");
            acpi_memcpy(result.buffer, method + state->pc, initial_size);
            state->pc += initial_size;

            if(exec_result_mode == LAI_DATA_MODE || exec_result_mode == LAI_OBJECT_MODE)
            {
                acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
                acpi_move_object(opstack_res, &result);
            }else
                LAI_ENSURE(exec_result_mode == LAI_EXEC_MODE);
            acpi_free_object(&result);
            break;
        }
        case PACKAGE_OP:
        {
            state->pc++;

            // Size of the package initializer.
            // Note that not all elements of the package need to be initialized.
            size_t encoded_size;
            state->pc += acpi_parse_pkgsize(method + state->pc, &encoded_size);

            // The number of elements of the package.
            int num_ents = method[state->pc];
            state->pc++;

            acpi_stackitem_t *pkg_item = acpi_exec_push_stack_or_die(state);
            pkg_item->kind = LAI_PKG_INITIALIZER_STACKITEM;
            pkg_item->opstack_frame = state->opstack_ptr;
            pkg_item->pkg_index = 0;
            pkg_item->pkg_end = opcode_pc + encoded_size + 1;
            pkg_item->pkg_result_mode = exec_result_mode;

            acpi_object_t *opstack_pkg = acpi_exec_push_opstack_or_die(state);
            opstack_pkg->type = ACPI_PACKAGE;
            opstack_pkg->package = acpi_calloc(num_ents, sizeof(acpi_object_t));
            opstack_pkg->package_size = num_ents;
            break;
        }

        case (EXTOP_PREFIX << 8) | SLEEP_OP:
            acpi_exec_sleep(method, state);
            break;

        /* A control method can return literally any object */
        /* So we need to take this into consideration */
        case RETURN_OP:
        {
            state->pc++;
            acpi_object_t result = {0};
            acpi_eval_operand(&result, state, method);

            // Find the last LAI_METHOD_CONTEXT_STACKITEM on the stack.
            int j = 0;
            acpi_stackitem_t *method_item;
            while(1) {
                method_item = acpi_exec_peek_stack(state, j);
                if(!method_item)
                    acpi_panic("Return() outside of control method()\n");
                if(method_item->kind == LAI_METHOD_CONTEXT_STACKITEM)
                    break;
                // TODO: Verify that we only cross conditions/loops.
                j++;
            }

            // Remove the method stack item and push the return value.
            if(state->opstack_ptr) // This is an internal error.
                acpi_panic("opstack is not empty before return\n");
            acpi_object_t *opstack_res = acpi_exec_push_opstack_or_die(state);
            acpi_move_object(opstack_res, &result);

            acpi_exec_pop_stack(state, j + 1);
            acpi_exec_update_context(state);
            break;
        }
        /* While Loops */
        case WHILE_OP:
        {
            size_t loop_size;
            state->pc++;
            state->pc += acpi_parse_pkgsize(&method[state->pc], &loop_size);

            acpi_stackitem_t *loop_item = acpi_exec_push_stack_or_die(state);
            loop_item->kind = LAI_LOOP_STACKITEM;
            loop_item->loop_pred = state->pc;
            loop_item->loop_end = opcode_pc + loop_size + 1;
            break;
        }
        /* Continue Looping */
        case CONTINUE_OP:
        {
            // Find the last LAI_LOOP_STACKITEM on the stack.
            int j = 0;
            acpi_stackitem_t *loop_item;
            while(1) {
                loop_item = acpi_exec_peek_stack(state, j);
                if(!loop_item)
                    acpi_panic("Continue() outside of While()\n");
                if(loop_item->kind == LAI_LOOP_STACKITEM)
                    break;
                // TODO: Verify that we only cross conditions/loops.
                j++;
            }

            // Keep the loop item but remove nested items from the exeuction stack.
            state->pc = loop_item->loop_pred;
            acpi_exec_pop_stack(state, j);
            break;
        }
        /* Break Loop */
        case BREAK_OP:
        {
            // Find the last LAI_LOOP_STACKITEM on the stack.
            int j = 0;
            acpi_stackitem_t *loop_item;
            while(1) {
                loop_item = acpi_exec_peek_stack(state, j);
                if(!loop_item)
                    acpi_panic("Break() outside of While()\n");
                if(loop_item->kind == LAI_LOOP_STACKITEM)
                    break;
                // TODO: Verify that we only cross conditions/loops.
                j++;
            }

            // Remove the loop item from the execution stack.
            state->pc = loop_item->loop_end;
            acpi_exec_pop_stack(state, j + 1);
            break;
        }
        /* If/Else Conditional */
        case IF_OP:
        {
            size_t if_size;
            state->pc++;
            state->pc += acpi_parse_pkgsize(method + state->pc, &if_size);

            // Evaluate the predicate
            acpi_object_t predicate = {0};
            acpi_eval_operand(&predicate, state, method);

            acpi_stackitem_t *cond_item = acpi_exec_push_stack_or_die(state);
            cond_item->kind = LAI_COND_STACKITEM;
            cond_item->cond_taken = predicate.integer;
            cond_item->cond_end = opcode_pc + if_size + 1;

            if(!cond_item->cond_taken)
                state->pc = cond_item->cond_end;

            break;
        }
        case ELSE_OP:
            acpi_panic("Else() outside of If()\n");
            break;

        // "Simple" objects in the ACPI namespace.
        case NAME_OP:
            acpi_exec_name(method, ctx_handle, state);
            break;
        case BYTEFIELD_OP:
            acpi_exec_bytefield(method, ctx_handle, state);
            break;
        case WORDFIELD_OP:
            acpi_exec_wordfield(method, ctx_handle, state);
            break;
        case DWORDFIELD_OP:
            acpi_exec_dwordfield(method, ctx_handle, state);
            break;

        // Scope-like objects in the ACPI namespace.
        case SCOPE_OP:
        {
            state->pc++;

            size_t encoded_size;
            state->pc += acpi_parse_pkgsize(method + state->pc, &encoded_size);
            char name[ACPI_MAX_NAME];
            state->pc += acpins_resolve_path(ctx_handle, name, method + state->pc);

            acpi_nsnode_t *node = acpins_create_nsnode_or_die();
            node->type = ACPI_NAMESPACE_SCOPE;
            acpi_strcpy(node->path, name);
            acpins_install_nsnode(node);

            acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
            item->kind = LAI_POPULATE_CONTEXT_STACKITEM;
            item->ctx_handle = node;
            item->ctx_limit = opcode_pc + encoded_size + 1;
            acpi_exec_update_context(state);
            break;
        }
        case (EXTOP_PREFIX << 8) | DEVICE:
        {
            state->pc += 2;

            size_t encoded_size;
            state->pc += acpi_parse_pkgsize(method + state->pc, &encoded_size);
            char name[ACPI_MAX_NAME];
            state->pc += acpins_resolve_path(ctx_handle, name, method + state->pc);

            acpi_nsnode_t *node = acpins_create_nsnode_or_die();
            node->type = ACPI_NAMESPACE_DEVICE;
            acpi_strcpy(node->path, name);
            acpins_install_nsnode(node);

            acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
            item->kind = LAI_POPULATE_CONTEXT_STACKITEM;
            item->ctx_handle = node;
            item->ctx_limit = opcode_pc + encoded_size + 2;
            acpi_exec_update_context(state);
            break;
        }
        case (EXTOP_PREFIX << 8) | PROCESSOR:
            state->pc += acpins_create_processor(ctx_handle, method + state->pc);
            break;
        case (EXTOP_PREFIX << 8) | THERMALZONE:
        {
            state->pc += 2;

            size_t encoded_size;
            state->pc += acpi_parse_pkgsize(method + state->pc, &encoded_size);
            char name[ACPI_MAX_NAME];
            state->pc += acpins_resolve_path(ctx_handle, name, method + state->pc);

            acpi_nsnode_t *node = acpins_create_nsnode_or_die();
            node->type = ACPI_NAMESPACE_THERMALZONE;
            acpi_strcpy(node->path, name);
            acpins_install_nsnode(node);

            acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
            item->kind = LAI_POPULATE_CONTEXT_STACKITEM;
            item->ctx_handle = node;
            item->ctx_limit = opcode_pc + encoded_size + 2;
            acpi_exec_update_context(state);
            break;
        }

        // Leafs in the ACPI namespace.
        case METHOD_OP:
            state->pc += acpins_create_method(ctx_handle, method + state->pc);
            break;
        case ALIAS_OP:
            state->pc += acpins_create_alias(ctx_handle, method + state->pc);
            break;
        case (EXTOP_PREFIX << 8) | MUTEX:
            state->pc += acpins_create_mutex(ctx_handle, method + state->pc);
            break;
        case (EXTOP_PREFIX << 8) | OPREGION:
        {
            state->pc += 2;
            char name[ACPI_MAX_NAME];
            state->pc += acpins_resolve_path(ctx_handle, name, method + state->pc);

            // First byte identifies the address space (memory, I/O ports, PCI, etc.).
            int address_space = method[state->pc];
            state->pc++;

            // Now, parse the offset and length of the opregion.
            acpi_object_t disp = {0};
            acpi_object_t length = {0};
            acpi_eval_operand(&disp, state, method);
            acpi_eval_operand(&length, state, method);

            acpi_nsnode_t *node = acpins_create_nsnode_or_die();
            acpi_strcpy(node->path, name);
            node->op_address_space = address_space;
            node->op_base = disp.integer;
            node->op_length = length.integer;
            acpins_install_nsnode(node);
            break;
        }
        case (EXTOP_PREFIX << 8) | FIELD:
            state->pc += acpins_create_field(ctx_handle, method + state->pc);
            break;
        case (EXTOP_PREFIX << 8) | INDEXFIELD:
            state->pc += acpins_create_indexfield(ctx_handle, method + state->pc);
            break;

        case ARG0_OP:
        case ARG1_OP:
        case ARG2_OP:
        case ARG3_OP:
        case ARG4_OP:
        case ARG5_OP:
        case ARG6_OP:
        {
            if(exec_result_mode == LAI_OBJECT_MODE
                    || exec_result_mode == LAI_TARGET_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_ARG_NAME;
                result->index = opcode - ARG0_OP;
            }
            state->pc++;
            break;
        }

        case LOCAL0_OP:
        case LOCAL1_OP:
        case LOCAL2_OP:
        case LOCAL3_OP:
        case LOCAL4_OP:
        case LOCAL5_OP:
        case LOCAL6_OP:
        case LOCAL7_OP:
        {
            if(exec_result_mode == LAI_OBJECT_MODE
                    || exec_result_mode == LAI_TARGET_MODE)
            {
                acpi_object_t *result = acpi_exec_push_opstack_or_die(state);
                result->type = ACPI_LOCAL_NAME;
                result->index = opcode - LOCAL0_OP;
            }
            state->pc++;
            break;
        }

        case STORE_OP:
        case NOT_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = LAI_TARGET_MODE;
            op_item->op_arg_modes[2] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }
        case ADD_OP:
        case SUBTRACT_OP:
        case MULTIPLY_OP:
        case AND_OP:
        case OR_OP:
        case XOR_OP:
        case SHR_OP:
        case SHL_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[2] = LAI_TARGET_MODE;
            op_item->op_arg_modes[3] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }
        case DIVIDE_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[2] = LAI_TARGET_MODE;
            op_item->op_arg_modes[3] = LAI_TARGET_MODE;
            op_item->op_arg_modes[4] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }

        case INCREMENT_OP:
        case DECREMENT_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_TARGET_MODE;
            op_item->op_arg_modes[1] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }

        case LNOT_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }
        case LAND_OP:
        case LOR_OP:
        case LEQUAL_OP:
        case LLESS_OP:
        case LGREATER_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[2] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }

        case INDEX_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_TARGET_MODE;
            op_item->op_arg_modes[1] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[2] = LAI_TARGET_MODE;
            op_item->op_arg_modes[3] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }
        case DEREF_OP:
        case SIZEOF_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            op_item->op_arg_modes[0] = LAI_OBJECT_MODE;
            op_item->op_arg_modes[1] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc++;
            break;
        }
        case (EXTOP_PREFIX << 8) | CONDREF_OP:
        {
            acpi_stackitem_t *op_item = acpi_exec_push_stack_or_die(state);
            op_item->kind = LAI_OP_STACKITEM;
            op_item->op_opcode = opcode;
            op_item->opstack_frame = state->opstack_ptr;
            // TODO: There should be a NAME_MODE that allows only names to be parsed.
            op_item->op_arg_modes[0] = LAI_DATA_MODE;
            op_item->op_arg_modes[1] = LAI_TARGET_MODE;
            op_item->op_arg_modes[2] = 0;
            op_item->op_result_mode = exec_result_mode;
            state->pc += 2;
            break;
        }

        default:
            acpi_panic("unexpected opcode in acpi_exec_run(), sequence %02X %02X %02X %02X\n",
                    method[state->pc + 0], method[state->pc + 1],
                    method[state->pc + 2], method[state->pc + 3]);
        }
    }

    return 0;
}

int acpi_populate(acpi_nsnode_t *parent, void *data, size_t size, acpi_state_t *state) {
    acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
    item->kind = LAI_POPULATE_CONTEXT_STACKITEM;
    item->ctx_handle = parent;
    item->ctx_limit = size;
    acpi_exec_update_context(state);

    state->pc = 0;
    state->limit = size;
    int status = acpi_exec_run(data, state);
    if(status)
        acpi_panic("acpi_exec_run() failed in acpi_populate()\n");
    return 0;
}

// acpi_exec_method(): Finds and executes a control method
// Param:    acpi_nsnode_t *method - method to execute
// Param:    acpi_state_t *state - execution engine state
// Return:    int - 0 on success

int acpi_exec_method(acpi_nsnode_t *method, acpi_state_t *state)
{
    // Check for OS-defined methods.
    if(method->method_override)
        return method->method_override(state->arg, &state->retvalue);

    // Okay, by here it's a real method.
    //acpi_debug("execute control method %s\n", method->path);
    acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
    item->kind = LAI_METHOD_CONTEXT_STACKITEM;
    item->ctx_handle = method;
    acpi_exec_update_context(state);

    state->pc = 0;
    state->limit = method->size;
    int status = acpi_exec_run(method->pointer, state);
    if(status)
        return status;

    /*acpi_debug("%s finished, ", method->path);

    if(state->retvalue.type == ACPI_INTEGER)
        acpi_debug("return value is integer: %d\n", state->retvalue.integer);
    else if(state->retvalue.type == ACPI_STRING)
        acpi_debug("return value is string: '%s'\n", state->retvalue.string);
    else if(state->retvalue.type == ACPI_PACKAGE)
        acpi_debug("return value is package\n");
    else if(state->retvalue.type == ACPI_BUFFER)
        acpi_debug("return value is buffer\n");*/

    if(state->opstack_ptr != 1) // This would be an internal error.
        acpi_panic("expected exactly one return value after method invocation\n");
    acpi_object_t *result = acpi_exec_get_opstack(state, 0);
    acpi_move_object(&state->retvalue, result);
    acpi_exec_pop_opstack(state, 1);
    return 0;
}

// Evaluates an AML expression recursively.
// TODO: Eventually, we want to remove this function. However, this requires refactoring
//       acpi_exec_run() to avoid all kinds of recursion.

void acpi_eval_operand(acpi_object_t *destination, acpi_state_t *state, uint8_t *code) {
    int opstack = state->opstack_ptr;

    acpi_stackitem_t *item = acpi_exec_push_stack_or_die(state);
    item->kind = LAI_EVALOPERAND_STACKITEM;
    item->opstack_frame = opstack;

    int status = acpi_exec_run(code, state);
    if(status)
        acpi_panic("acpi_exec_run() failed in acpi_eval_operand()\n");

    if(state->opstack_ptr != opstack + 1) // This would be an internal error.
        acpi_panic("expected exactly one opstack item after operand evaluation\n");
    acpi_object_t *result = acpi_exec_get_opstack(state, opstack);
    acpi_load_operand(state, result, destination);
    acpi_exec_pop_opstack(state, 1);
}

// acpi_exec_sleep(): Executes a Sleep() opcode
// Param:    void *code - opcode data
// Param:    acpi_state_t *state - AML VM state

void acpi_exec_sleep(void *code, acpi_state_t *state)
{
    state->pc += 2; // Skip EXTOP_PREFIX and SLEEP_OP.

    acpi_object_t time = {0};
    acpi_eval_operand(&time, state, code);

    if(!time.integer)
        time.integer = 1;

    acpi_sleep(time.integer);
}




