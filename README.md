# RTOS-Essentials

## Updates

### Week 1 – 13/09/2025  
feat: Add RTOS task demos for bare-metal vs RTOS comparison  
- Implemented 3 concurrent tasks (LED blink, high/low priority prints)  
- Demonstrates FreeRTOS priority-based scheduling  
- Complete Arduino Uno setup with detailed comments  

### Week 2 – 20/09/2025  
feat: Add delay handling demo (blocking vs non-blocking)  
- Bare-metal version: `delay()` blocks all tasks, halts system concurrency  
- FreeRTOS version: `vTaskDelay()` allows true multitasking  
- Demonstrates LED toggling every 5s + continuous serial prints every 500ms  
- Clear timestamps in output show difference in task scheduling  

### Week 3 – 27/09/2025  
feat: Add task communication demo (global variable vs FreeRTOS queue)  
- **Global Variable Demo:** Increment + Reset tasks overwrite each other, Print task sees inconsistent values (race condition risk)  
- **Queue Demo:** Increment + Reset tasks push data safely into a queue, Print task retrieves values in FIFO order without corruption  
- Demonstrates how FreeRTOS queues solve the limitations of globals by providing atomic, ordered, and synchronized task communication  
- Clear serial logs highlight the contrast: globals cause overwrites, queues guarantee message delivery  
