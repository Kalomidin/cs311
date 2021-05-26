# Project 3. MIPS Pipelined Simulator
Skeleton developed by CMU,
modified for KAIST CS311 purpose by THKIM, BKKIM and SHJEON.

## Instructions
There are three files you may modify: `util.h`, `run.h`, and `run.c`.

### 1. util.h

We have setup the basic CPU\_State that is sufficient to implement the project.
However, you may decide to add more variables, and modify/remove any misleading variables.

### 2. run.h

You may add any additional functions that will be called by your implementation of `process_instruction()`.
In fact, we encourage you to split your implementation of `process_instruction()` into many other helping functions.
You may decide to have functions for each stages of the pipeline.
Function(s) to handle flushes (adding bubbles into the pipeline), etc.

### 3. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` function to simulate a `cycle` of the pipelined simulator.
Each `cycle()` the pipeline will advance to the next instruction (if there are no stalls/hazards, etc.).
Your internal register, memory, and pipeline register state should be updated according to the instruction
that is being executed at each stage.


----
# Newly Defined Fields
- `ID_EX`
`ID_EX_OP_CODE` -> OPCODE of the instruction
`ID_EX_FUNC` -> Function of the instruction
`ID_EX_RS` -> `rs` value of the instruction
`ID_EX_RT` -> `rt` value of the instruction
`ID_EX_RD` -> `rd` value of the instruction
`EX_STALL` -> Stall due to `lw` in the `ID` stage
`J_FLUSH` -> jump flush in the `id` stage
`J_TARGET` -> Jump's target value 
`ID_EX_PREV_NPC` -> value to save value of the npc of the id stage. This is used to handle `ex_stall`

- `EX_MEM`
`EX_MEM_READ` -> boolean that indicates read or not to the memory
`EX_MEM_WRITE` -> boolean that indicates to write or not to the memory
`EX_MEM_REG_WRITE` -> boolean indicating to write to the register or not 

- `MEM_WB`
`MEM_WB_REG_WRITE` -> boolean indicating to write to the register or not
`MEM_WB_MEM_RG` -> boolean idnicating to write memory or alu output