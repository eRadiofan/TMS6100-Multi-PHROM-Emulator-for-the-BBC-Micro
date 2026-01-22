#BASIC Utilities

Here are some BASIC2 utility programs intended to run on the BBC Micro model A/B, they are provided as text and in tokenised form. A description of each follows:

1. SP-RAM - Speaks a phrase from RAM to test a standalone TMS5220. Taken from the Acorn speech system user guide, it saves some typing.

2. SPHRASE - Speaks phrases from PHROMs. Enter the PHROM bank number followed by the phrase number and it speaks. Entering -1 will speak all phrases in the selected PHROM. Press the return key to repeat the previous selection. The program contains index data for PHROM bank 2 (VM61004 which is non-indexed) so that phrases are easy to select. It will detect the TI-99 build option and function accordingly. For reference and experimentation, the program contains an indirectly addressed speak function as well as a directly addressed speak function.

3. SBOUNDS - This is a test program to check various edge cases when using the milti-PHROM. In particular:

>* Reading data after the end of a recognised PHROM bank into an unmapped address space, releases the data line and continues incrementing the address pointer.

>* Reading data from the end of an unmapped PHROM bank, upon entering a recognised address space, activates the data line an returns correct data.

>* For the clock PHROM (bank 4), containing only 4K of data, check that the end of the 4K is padded with 0s and that reading past the 4K boundary, releases the data line.

>* Check that reading past the end of PHROM bank 15 wraps around to PHROM bank 0.

>* Check that the multi-PHROM does not interfere with other PHROMs when indirect addressing is used. If an indirect read is received while the address points to some other PHROM, this multi-PHROM transitions to it's initial state, waiting for the next address. Indirect addressing will not work across PHROM boundaries.
