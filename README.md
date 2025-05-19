<a name="readme-top"></a>

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#requirements">Requirements</a></li>
        <li><a href="#dependencies">Dependencies</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li>
      <a href="#development">Development</a>
      <ul>
        <li><a href="#using-makefile">Using Makefile</a></li>
        <li><a href="#building-locally">Building Locally</a></li>
        <li><a href="#using-docker">Using Docker</a></li>
      </ul>
    </li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

## About The Project

Contexto is a word-guessing game where players have to find the secret word with unlimited guesses and hints.

## Getting Started

### Requirements

- Local development:

  - Compiler with C++23 support
  - Python 3.10+ with venv support
  - Node.js 18+ and npm
  - CMake 3.20+
  - Ninja or Make build system

- Docker development:
  - Docker and Docker Compose

### Dependencies

- Backend:

  - [userver](https://github.com/userver-framework/userver) - High-performance C++ microservice framework
  - [Eigen](https://github.com/PX4/eigen) - C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms.

  - Various system libraries (automatically installed via `install-deps.sh` or Docker)

  - Embeddings. They must be downloaded separately and extracted into backend/assets directory. Embeddings must have Universal Tags tageset and use Continuous Skipgram algorithm. Default embeddings that is currently used(also must be downloaded separately) is [ruwikiruscorpora-nobigrams_upos_skipgram_300_5_2018](https://rusvectores.org/static/models/rusvectores4/unigrams/ruwikiruscorpora-nobigrams_upos_skipgram_300_5_2018.vec.gz) from [RusVectōrēs](https://rusvectores.org).

- Frontend:
  - React-based web application
  - Various npm packages (see `frontend/package.json`)

### Installation

#### Quick Start with Docker (Recommended)

```sh
# Clone the repository with submodules
git clone https://github.com/RexarX/Contexto.git
cd Contexto
git submodule update --init --recursive

# Build and start with Docker
make docker-build
make docker-up
```
