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