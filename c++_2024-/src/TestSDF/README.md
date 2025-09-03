# SDF test

![](summary.gif)

Here is some quick and simple setup to blend a raymarched SDF (signed distance fields) scene with a traditional rasterized one. The setup is heavily inspired by Iñigo Quilez's articles on the subject.

## Building notes

If you use vim, you can load the .vimrc in this project to set up commands for building and running the project via \<leader>bb and \<leader>rr, respectively.

For building this project, refer to [this very rough blurb](../../README.md). COMPILE_TARGET should be set to TEST_SDF.

## Breakdown

The SDF scene is roughly based from a very quick mock-up done in Adobe Neo:

![](breakdown_0.gif)

The head is a simple sphere, as are the eyes, which project their material onto the head. The beak is constructed using a cone, and the body uses two boxes, one for the main color and shape, and one for the projected material around the belt. The limbs are also box shapes.

![](breakdown_1.jpg)

The mirrors use the same setup as [the one detailed in this test](../testMirrors/README.md), and don't really add much to the scene, since it's not reasonable to render the SDF scene with as many passes as a recursive mirror scene demands. 
The SDF scene could handle its own reflections, but it wouldn't have a way to blend the depth with the rasterized scene, since we lose the depth of every reflection after each mirror pass. Regardless, the mirror scene is there and the SDF scene does render on each mirror pass, however it is at least culled using the camera's frustum.

This test also uses a new zero-memory immediate mode UI, based on Casey Muratori's and Sean Barrett's work:

![](breakdown_2.gif)