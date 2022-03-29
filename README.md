# Nietsneflow (temporary name)

The main purpose of this projects to re-implement Wolfenstein3D engine.

## Quick review of Wolfenstein3D

### Main presentation

Wolfenstein3d was released on the year 1992 by Id Software. The game take place during the WWII.
The player incarnate a prisonner who have to escape from the wolfenstein castle, which is a prison. 
The charachter is in subjective view. The game is divided in several levels. The levels is complete when we reach a 
precise place, or we kill a boss.
There are many static elements and mobile ennemies (AI) in the environment.
There are many doors that we can open, and many secret areas hidden in that what appear like simple walls.
All along the adventure the player have to pick up many objects:
Like weapons, ammunations, gold and food.
The game allow to save at any time.

### Technical details                                                                                                                                                                                                                             

The game is one of the first FPS in the history, the hardware of the time did not allow 3d render.
So the engine simulate it in a 2d logic. The used technique was called raycasting. 
The levels in wolfenstein3d were respresented on a 2d map, to display the elements the engine calculate the distance between the player and
the concerned element. Depending on the angle and distance the element was displayed in consequences.

## Project screenshots

![image](https://user-images.githubusercontent.com/3542204/160563140-75c81f44-57fb-484a-ae4d-260374feb98a.png)
![image](https://user-images.githubusercontent.com/3542204/160563911-1595277c-a535-498e-aa07-6cb803cf9de2.png)
![image](https://user-images.githubusercontent.com/3542204/160566331-8e693eda-27d0-4fe2-8b56-187ab5f81c85.png)
