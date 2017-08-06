# ![Cartographer Logo](/icon32.png) Cartographer
Framework collection for rapid game development (targeted for and partially based on Game-Jam code).
The purpose and features of all modules are documented below.

The cartographer framework consists of multiple modules which are included as independent (orphaned) branches. Some of them are more related to games and other can be useful in different projects too. Some modules depend on other libraries (Epsilon, Chi-Noise). The interdepence between the modules looks as follows:

![Cartographer Dependencies](https://github.com/Jojendersie/Cartographer/cartographerdeps.svg)

## Paper

Containers and low level constructs for general programming:
* Different forms of experimentally smart pointers
* A logging system
* HashSet and HashMap with Robinhood-Hashing (faster insertion/removal than std:: containers, slightly slower search times due to meta data which polutes the cache)
* HashPriorityQueue: a combined hashset and priority queue for fast updates of arbitrary elements

## Charcoal

Simple sprite and instanced OpenGL renderer including a high quality (distance based) image **font rendering**.
This was the first module of the Cartographer framework based on Game-Jam code. The render is not designed for ultimate performance and there is room for improvements. However, font rendering was added later and is much better than expected when starting to write it.
* Sprite Rendering
* Instances of arbitrary 2D/3D geometry with similar interfaces to old OpenGL.
* Font Rendering

## GUI

A render GUI. The current implementation is based on Charcoal, but has some interfaces designed to exchange the renderes. There is currently no optimization like caching which should be included in feature but also might break the interfaces.
* Widgets: Frames, Labels, Images, Buttons, Sliders and Checkboxes so far
* Helpers (like Bezier-Connectors) for graph based GUI layouts

## Map

A collection of algorithms to help with the development of 2D maps. This part is heavily template and functor based to allow very different kinds of maps to be implemented. Most algorithms are supported for quad- and hex-grids by defining variable neighborhood schemes.
* Grid: the base class for a row based map storage
	* there is a deprecated sparse variant which is currently not developed further for performance reasons
* Algorithms
	* A* way search
	* Connected component searches
	* Sampling algorithms (Populate)
* Helper functions like iterators