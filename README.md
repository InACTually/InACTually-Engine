<img src="assets/design/splash.png" alt="InACTually Engine Splash" width="250"/>

# InACTually Engine
a MediaServer for driving all technology in\
interactive theater for actual acts


InACTually is (or wants to be) an artistic tool and consists of the Engine and the Stage. InACTually allows artists to realize their multimedia and multimodal concepts autonomously. The underlying spatiality of performing arts is used to design an intuitive and easy-to-use interface, which introduces direct manipulation as well as immersive applications in Mixed Reality.

<br>

## 🎢 What InACTually actually is
... or what it wants to be.
We should definitively path this out.

InACTually started 2021 as tiny tool made by Lars Engeln and Fabian Töpfer that suits their needs to create spatial media installations and interactive moments in theater productions. So it got bigger and bigger over time, sometimes without really any time to refurbish. Thereby, some parts might be a bit deprecated, but we are tidying up :)

The Engine is InACTually's media server, that should be able to run a show or media installations on its own. Its core idea is to think everything spatially. Thereby, it has RoomNodes that express something in space like devices or tracked objects. According to that, there are ProcessingNode, which can be used to orchestrate more complex behaviour in the node-link diagram. 

[The Stage](https://github.com/InACTually/InACTually-Stage) (in another repo) is InACTually's frontend, that should be nicer to use and should only give instructions to the Engine what to do. The Stage is meant to be hosted in a venue, to easily access it via a tablet or a Mixed-Reality headset.

[The NodeCreator](https://github.com/InACTually/InACTually-NodeCreator) (in another repo) is InACTually's helper tool to easily create and integrate processing nodes into the InACTually Engine's codebase. The NodeCreator is a submodule for this repository.

<br>

## 🎭 Gallery

We would like to build up a Gallery to showcase how InACTually is been used in all the lovely projects. Please contact us, if you use InACTually (all tools or one of them) in your projects.
And think about to contribute to the Gallery. \
Some impressions from projects by Fabian Töpfer and Lars Engeln using InACTually:

<img src="./gallery/InACTually-Impressions_FabianTöpfer-LarsEngeln.png" alt="InACTually Gallery Impressions - Fotos by Lars Engeln" />

<br>

## 🛠️ Setup – How to compile

If you want to have the [NodeCreator](https://github.com/InACTually/InACTually-NodeCreator) with the Engine, you need to clone `--recursive`, as it is a submodule. \
To make the InACTually Engine compile... pray and see the [SETUP.md](./SETUP.md) for some details.

<br>

## ✒️ Contributing
Whether you are fixing a bug, suggesting an improvement, or just asking a question, you are helping make this better. Best is always to contact us, but you find more information in the [CONTRIBUTING.md](CONTRIBUTING.md).

<br>

## 🧩 Third-Party Notice and Credits

The InACTually Engine project uses external resources licensed under open-source terms. \
You can find the list of frameworks, code snippets, models, and other material in the [THIRD_PARTY.md](THIRD_PARTY.md).

<br>

## 📜 License
The InACTually Engine is licensed under the MIT License, see the [LICENSE](LICENSE). \
Contact us, if you use InACTually (all tools or one of them) in your projects. 
Would be nice to see, what lovely things you are creating :)