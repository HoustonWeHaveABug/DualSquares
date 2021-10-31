# DualSquares

Solution to Reddit Daily Programmer Challenge #368 https://www.reddit.com/r/dailyprogrammer/comments/9z3mjk/20181121_challenge_368_intermediate_singlesymbol/.

The program dual_squares is the inital solution for the challenge. It reads 3 parameters on standard input:
- Order (>= 2)
- Maximum number of single squares (>= 0)
- Period of solutions counter display (>= 1)

The program erickson_matrix is an evolution that can manage more than 2 colors. It is implementing a spiral out scan of the grid and enhanced symmetry management that allows to prove there is no solution with 0 single squares for 2 colors and order = 15 in 4h15m on my current desktop computer. It reads 4 parameters on standard input:
- Number of colors (>= 2)
- Order (>= 2)
- Maximum number of single squares (>= 0)
- Number of single squares after which the program tries only one color per cell (>= 0, <= Maximum number of single squares)
