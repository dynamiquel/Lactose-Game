# About Lactose
Lactose is a **full-stack farm simulation game** project designed to explore and demonstrate a **robust, scalable backend architecture** for game development. Unlike traditional game development that often tightly couples game logic with the client, Lactose adopts a mobile-game-inspired approach where nearly all core game mechanics and data persistence reside on the server.

This project showcases a modern microservices architecture built with **ASP.NET Core**, providing a scalable and resilient foundation for a dynamic game experience where real-time updates and complex simulations are handled **server-side**.

# The Journey
Having spent a considerable amount of time in traditional game development focusing on client-side programming, Lactose was conceived as a deliberate and intensive deep dive into **modern backend** development. My primary goal was to gain hands-on expertise with technologies and paradigms that were largely new to me before this project.

Specifically, Lactose has allowed me to:
- **Master ASP.NET Core**: From foundational concepts to building robust **APIs**.
- **Embrace Microservices**: Designing, implementing, and managing loosely coupled services.
- **Navigate Containerization**: Gaining practical experience with **Docker** and Docker Compose.
- **Tackle Orchestration**: Learning the complexities and power of **Kubernetes**, specifically K3s (a challenging but rewarding journey!).
- **Work with NoSQL Databases**: Proficiently using **MongoDB** for flexible and scalable data storage.
- **Implement Diverse Communication Patterns**: Understanding and applying **RESTful** APIs for synchronous communication and **MQTT** for asynchronous pub/sub messaging.
- **Develop Custom Tooling**: Creating an IDL-driven code generation tool [Catalyst](https://github.com/dynamiquel/Catalyst) to streamline API development, akin to gRPC.

This project represents a **significant personal and professional growth** in my backend capabilities, proving my ability to **architect, implement, and manage complex distributed systems**.

# Architecture Overview
Lactose is split into two primary repositories, reflecting its clear frontend-backend separation:
## Lactose-Game
The game client is developed using **Unreal Engine 5**, responsible for rendering the game world, handling player input, and visualizing game state. All critical game logic, data, and state management are offloaded to the backend. The Unreal client communicates with the backend via **RESTful APIs** and subscribes to real-time events via **MQTT** for dynamic updates.
[Lactose-Game Repository](https://github.com/dynamiquel/Lactose-Game)
## Lactose-Server
The heart of Lactose lies in its **ASP.NET Core microservices backend**. This highly **decoupled** architecture allows for independent development, deployment, and scaling of various game features.
[Lactose-Server Repository](https://github.com/dynamiquel/Lactose-Server)

## Services
Each microservice is designed with a **single responsibility**, communicating via defined APIs and event streams.
### Identity
- Manages user **authentication** and **profiles**.
- Leverages **JSON Web Tokens** for secure, stateless authentication.
### Config
- A **dynamic, live configuration** system accessible via key-value pairs.
- Enables **real-time game adjustments** and updates without requiring client patches.
### Simulation
- The core **farming logic** service.
- Handles crucial game mechanics like planting, crop growth simulation, seeding, harvesting, and fertilisation.
### Economy
- Manages the **in-game item economy**, including player inventories and vendor shops.
- Facilitates buying items from vendors and selling items.
### Tasks
- An **achievement and progression** system.
- Tracks player actions by subscribing to MQTT events fired by other services.
- Awards players with rewards upon task completion.

## Communication Patterns
### Synchronous Communication (REST & Catalyst)
- Primarily uses **RESTful HTTP APIs** for synchronous requests (e.g., getting user data, buying an item).
- Some REST communication is powered by my other project, [Catalyst](https://github.com/dynamiquel/Catalyst), a custom-built **Interface Definition Language** with code generation capabilities. Catalyst simplifies API development and ensures consistency, similar in principle to gRPC, but leveraging standard HTTP REST.
### Asynchronous Communication (MQTT)
- Utilises **MQTT** for **publish-subscribe** eventing between services.
- This enables **real-time updates** and **loose coupling**, where services react to events without direct knowledge of the event producers. For instance, the Tasks service listens for events from Simulation and Economy to track player progress.

## Database 
**MongoDB** is the primary NoSQL database for all microservices, chosen for its flexibility and scalability in handling diverse game data. Each service generally manages its own data stores for true microservice autonomy.

## Metrics
Metrics are provided by the microservices, tracked by **Prometheus** and displayed using **Grafana**.

## Containerization & Orchestration
The entire backend is designed for deployment within **containerised** environments, ensuring **portability** and ease of management.
### Docker Compose
The initial setup for local development and simplified multi-container deployment. This is the quickest way to get all backend services running.
### K3S (Kubernetes)
The backend has been adapted for deployment on [K3s](https://k3s.io/), a lightweight **Kubernetes** distribution. This demonstrates readiness for more **robust, scalable, and production**-like orchestration environments, albeit a challenging but highly rewarding learning experience in distributed systems management.

# Getting started
## Test server
You can give Lactose a go by downloading the **game client** in the [Lactose-Game Repository](https://github.com/dynamiquel/Lactose-Game). This client will automatically connect to the **production** Lactose Kubernetes deployment.
### Verifying the server is live
Sometimes the production server may not be live, but you can verify that it is by accessing this [link](https://lactose2.mookrata.ovh/).
## Self-host
There is currently no proper support for getting Lactose up and running in your own setup.
### Lactose Server
The backend server can be forked but it lacks a few secrets to get it up and running, and there is no documentation at the moment to overcome this.
### Lactose Game
The frontend game client can be forked but it lacks any binary assets at this moment in time, making the game unplayable.

# Contributing
Lactose is a personal portfolio project. While direct contributions are not actively sought, feel free to fork the repository, experiment, and use it as a reference for your own learning!
