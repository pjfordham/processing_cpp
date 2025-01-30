<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]


<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h3 align="center">C++ Mode for Processing - Proce++ing </h3>

  <p align="center">
    Clean room port of the Processing Framework to C++!
    <br />
    <br />
    <a href="https://github.com/pjfordham/processing_cpp/issues">Report Bug</a>
    ·
    <a href="https://github.com/pjfordham/processing_cpp/issues">Request Feature</a>
  </p>
</div>



<!-- ABOUT THE PROJECT -->
## About The Project

Inspired by the need for a friendly and simple way to display graphics in C++ harking back to the halcyon days of Borland C++. The Processing
API seemed to be a good fit.

The first step was implementing enough of the API to run the Clock example. Support for other examples was added incrementally from there and
now most of them work at comparable performance to their Java versions. With C++ and Java sharing similar syntax we are actually able to run
some of the examples unmodified, albeit with a little preprocessor trickery.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With
[![C++20](https://img.shields.io/badge/C%2B%2B%2020-blue)](https://en.cppreference.com/w/cpp/20)
[![OpenGL](https://img.shields.io/badge/OpenGL-blue)](https://www.opengl.org/)
[![GLAD](https://img.shields.io/badge/GLAD-blue)](https://github.com/Dav1dde/glad)
[![GLFW](https://img.shields.io/badge/GLFW-blue)](https://www.glfw.org/)
[![fmtlib](https://img.shields.io/badge/fmtlib-blue)](https://github.com/fmtlib/fmt)
[![freetype](https://img.shields.io/badge/freetype-blue)](https://www.freetype.org/)
[![tess2](https://img.shields.io/badge/tess2-blue)](https://github.com/diatomic/tess2)


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This is an example of how you may give instructions on setting up your project locally.
To get a local copy up and running follow these simple example steps.

### Prerequisites

Placeholder for detailed list of what you need beyond the git submodules.

### Installation


1. Clone the repo
   ```sh
   git clone https://github.com/pfordham/processing_cpp
   ```
2. Build with cmake
   ```sh
   mkdir release
   cd release
   cmake ..
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- USAGE EXAMPLES -->
## Usage

After building just run any of the executables built from the examples directories.

To get a taste of what to expect see <a href="https://processing.org/examples/">visit</a>, the original framwork's examples pages. Most of these examples work identically to their Java and Javascript counterparts.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

See the [open issues](https://github.com/pjfordham/processing_cpp/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.


<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

<a href="https://processing.org/">Processing</a>

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/pjfordham/processing_cpp.svg?style=for-the-badge
[contributors-url]: https://github.com/pjfordham/processing_cpp/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/pjfordham/processing_cpp.svg?style=for-the-badge
[forks-url]: https://github.com/pjfordham/processing_cpp/network/members
[stars-shield]: https://img.shields.io/github/stars/pjfordham/processing_cpp.svg?style=for-the-badge
[stars-url]: https://github.com/pjfordham/processing_cpp/stargazers
[issues-shield]: https://img.shields.io/github/issues/pjfordham/processing_cpp.svg?style=for-the-badge
[issues-url]: https://github.com/pjfordham/processing_cpp/issues
[license-shield]: https://img.shields.io/github/license/pjfordham/processing_cpp.svg?style=for-the-badge
[license-url]: https://github.com/pjfordham/processing_cpp/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/peterfordham

