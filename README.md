# embedded-game

Miniature embedded RPG in C for the ARM Cortex-A9 processor on DE1-SoC. It's kinda cringe and I could probably make the animations more epic, but that's a later problem. It doesn't have anything to do with real Pokemon, so please don't gouge your eyes out when Nasty Plot gives an additive damage buff or whatever.

If you wanna try it out at the risk of incurring brain damage from the lameness of the game, you can emulate it on CPUlator. Just make sure to paste the contents of the sprite header into the file containing the executable, because CPUlator doesnt let you drop multiple files in. As DJ Benjammin would say, "That. Is not. Cool."

So it's a useless game that isn't actually fun to play (I'm pretty sure it's rigged too), but just like Mihoyo, all the budget went into animation. The difference between
Genshin Impact and this thing is that their animations are actually good, and my game actually runs correctly.

**Instructions for people who are gutsy (and perhaps bored) enough to play the game:**

Press a SW 7-4 for charmander’s move, press a SW 3-0 for pikachu’s move (specific list below)

Deselect both switches to watch the moves play out (who goes first is randomly determined at game start time), HP bars update

Moves execute as long as at least one of the dudes is still making a move, so players can change their moves in the meanwhile

First pokemon to 0 hp loses, and you will get an end screen for the winner on a timer

Game restarts


**---Charmander moves---**

7 Flamethrower - Damage based on attack

6 Dragon Rage - Damage based on attack

5 Swords Dance - Raises attack

4 Metronome (randomly picks one of the other 3 moves)


**---Pikachu moves---**

3 Thunderbolt - Damage based on attack

2 Quick Attack - Damage based on attack

1 Nasty Plot - Raises attack

0 Metronome (randomly picks one of the other 3 moves)
