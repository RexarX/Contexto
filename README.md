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
        <li><a href="#how-to-build">How to build</a></li>
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
* Compiler with C++23 support

### Dependencies
* [userver](https://github.com/userver-framework/userver)

### How to build
1. Clone the repo
```sh
git clone https://github.com/RexarX/Contexto.git && git submodule update --init
```

2. Install dependencies

Install frontend dependencies
```sh
cd frontend
npm install
cd ..
```

Install backend dependencies
```sh
./install-deps.sh
```

3. Build backend
```sh
cd backend
./scripts/build.sh
```

## License

Distributed under the MIT license. See `LICENSE` for more information.

## Contact

RexarX - who727cares@gmail.com

PlatonFPS - platonfps@yandex.ru

pechikoo11 - shysha.shit@mail.ru

SindbaDRef - vnemaevv@mail.ru

<p align="right">(<a href="#readme-top">back to top</a>)</p>