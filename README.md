# simplecs

Simplecs (pronounced simplex) is a very simple C99 implementation of an Entity-Component-System (ECS).
ECS is a very useful design pattern/alternative to OOP that mainly finds use in video game development.
For more details about ECS, read...
I am aiming to be compileable with gcc and especially tcc.
Very inspired by flecs, which was created by Sanders Mertens.


Entities are indices (uint64_t).  
Components are structs.  
Systems are functions.  
The world loop iterates over systems.  


All contributions are welcome.  

Made by Gabriel Taillon...
