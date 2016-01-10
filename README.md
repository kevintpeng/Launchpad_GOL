# Launchpad_GOL
Project: Conway's Game of Life on the Texas-Instrument Launchpad micro-processor.

Class: SE 101, University of Waterloo, Fall 2015
Team Members: Kevin, Essa, Toby, Vlad, Shruti

Overview: 

Our team decided to recreate Conway’s Game of Life on the Launchpad Tiva C,
utilizing the Orbit Booster Pack. Conway’s Game of Life is a zero-player game, during
which cells, based on a set of rules, can live, multiply or die. Depending on the
game’s initiate state, the cells will form various patterns, which will be displayed
on the Booster Pack’s OLED display. However, by using the Booster Pack’s accelerometer
and potentiometer, our simulated life will be dependent, not only on the size of its
container, but also, on its surroundings. The potentiometer will vary the speed of
growth and death, while a shake of the Launchpad and Booster Pack will wipe all life
and start again. Finally, our Game of Life will feature an LED that will indicate the
current progression of life on the display. For example, the LED will become red, if
life has become sustainable.

Hardware Components:

- Launchpad Tiva C: for the computing power
- Orbit Booster pack: to display the game of life simulation by utilizing its own hardware components
- LEDs: indicates the current state of progression of the game. For example, the Booster Pack’s LED
will turn red, if life has become sustainable.
- 3-Axis Accelerometer: detects a shake of the system, which executes a restart feature of our game
- 128x32 Pixel OLED Display: Displays the pixels representing the cells in the Game of Life
- Analog Potentiometer: varies the frame rate of the game, speeding up or slowing down the progression of the game

Expected Challenges:

We anticipate various challenges while working on our project as it requires the use
multiple hardware components and relatively complex code. On the hardware side, we
will need to understand how the OLED screen, LEDs, the potentiometer and the
accelerometer work. On the software side, we will need write the code for the Game
of Life Simulation. When this is done, we will need to implement the speed variation
based on the potentiometer and set up a refresh function based on the accelerometer
value. We may find this final part challenging if we come across errors we do not
understand.
