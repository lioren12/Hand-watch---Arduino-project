# Hand watch---Arduino-project
Clock + Alarm + Stopper implemented over Digilent Basys MX3 MCU trainer board - C project.

This card contains a Microcontroller PIC32MX370 by Microchip, which includes MIPS CPU, Memories, timers, interrupts and DMA.

# Abstract
Clock display mode: SW0 = OFF, SW1 = OFF {"HH:MM:SS"}.

Clock time set mode: SW0 = ON, SW1 = OFF {Set time with BTNC(for H/M/S mode) , BTNR (for up) and BTNL (for down)}.

Alarm set mode: SW0 = OFF, SW1 = ON.

Alarm sound mode: SW7= ON {The alarm will ring for a minute}.

Stopper mode: SW0 = OFF, SW1 = OFF {Over the 7-segment display - "SS:QQ"}.


# Implementation details
1. The clock is built with one timer that runs during the entire alarm time.

1.1. For each mode of the slide switches, there is an interrupt called CurrentTime that takes care of the time updating
     Every millisecond except SET TIME mode (which responisble for setting the time).
     
1.2. In the SET TIME mode there is another interrupt called TimeForSetTime, which updates the clock
     for a Long PUSH operation. (Running the the time over the screen faster)
     
2. The stopwatch function works simultaneously with the clock operation at all times.

3. The stopwatch continues to run parallel to the clockwise position, even without displaying at that time.

4. The code contains libraries from the DEMOS library.
