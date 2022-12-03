# Interstellar
Course Project of Computer Graphics (浙江大学计算机图形学课程大作业)

## Introduction

This is a 3D game based on OpenGL, where the user can control a space ship to travel in the universe to view the scenery of stars, planets and other celestial bodies, and interact with the space stations.

Models and textures are from [Stellaris](https://www.paradoxinteractive.com/games/stellaris/). [Download](https://drive.google.com/file/d/1y_UD9X9rJGYQY0Etfk2Bv6yJDovEarTk/view?usp=share_link)

Control the space ship:

- `W`: accelerate
- `S`: decelerate
- `A`: turn left
- `D`: turn right
- `Space`: file missiles

## Methods

- Normal Mapping

  <img src="images/image-20221202231954215.png" alt="image-20221202231954215" style="zoom: 33%;" />

  Results:

  <p>
      <center>
          <img src="images/image-20221202232531407.png" style="zoom: 30%;">
          <img src="images/image-20221202232635820.png" style="zoom: 30%;">
      </center>
      <center><font color="#808080" size=2>Barren planet with (left) and without (right) normal mapping</font></center>
  </p>

  <p>
      <center>
          <img src="images/image-20221202232929398.png" style="zoom: 30%;">
          <img src="images/image-20221202232944255.png" style="zoom: 30%;">
      </center>
      <center><font color="#808080" size=2>Arid planet with (left) and without (right) normal mapping</font></center>
  </p>

  <p>
      <center>
          <img src="images/image-20221202233152219.png" style="zoom: 30%;">
          <img src="images/image-20221202233233339.png" style="zoom: 30%;">
      </center>
      <center><font color="#808080" size=2>Oceanic planet with (left) and without (right) normal mapping</font></center>
  </p>

  <p>
      <center>
          <img src="images/image-20221202233319021.png" style="zoom: 30%;">
          <img src="images/image-20221202233754749.png" style="zoom: 30%;">
      </center>
      <center><font color="#808080" size=2>Sentry Array with (left) and without (right) normal mapping</font></center>
  </p>

- Specular Mapping

  Different locations in the texture have different **reflective properties** and **lighting conditions**.

  <img src="images/image-20221202232146012.png" alt="image-20221202232146012" style="zoom: 50%;" />

  Results:

  <center><img src="images/Pic1.png" style="zoom: 50%;" /></center>

  <center><img src="images/image-20221202234118502.png" alt="image-20221202234118502" style="zoom: 33%;" /><img src="images/image-20221202234150218.png" alt="image-20221202234150218" style="zoom: 33%;" /></center>

- Particles

  We use particles to render stars more vividly:

  <center><img src="images/image-20221202234609867.png" style="zoom: 60%;"><img src="images/image-20221202234729695.png" style="zoom: 38%;"></center>

- Light Attenuation

  Objects farther from stars will appear darker.

- Terrain Generation Algorithm based on Perlin Noise

  We use tiles to model the geometry of the terrain.

  <img src="images/image-20221202235109746.png" alt="image-20221202235109746" style="zoom: 25%;" />

  Then we developed an algorithm based on Perlin noise to generate random terrain.

  Results:

  <img src="images/image-20221202235529301.png" alt="image-20221202235529301" style="zoom: 25%;" />

- Collision Detection

  We use *AABB* method for collision detection.

  <img src="images/image-20221202235705109.png" alt="image-20221202235705109" style="zoom:33%;" />

## Screenshot

<img src="images/Screenshot1.png" alt="Screenshot1" style="zoom: 67%;" />

<img src="images/image-20221203000113101.png" alt="image-20221203000113101" style="zoom: 50%;" />

<img src="images/image-20221203000318462.png" alt="image-20221203000318462" style="zoom:53%;" />
