/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack

  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.

  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main()
{

  int i;
  struct matrix *tmp;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  double step_3d = 50;
  double theta;

  //Lighting values here for easy access
  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  double light[2][3];
  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 255;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  double view[3];
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  //default reflective constants if none are set in script file
  struct constants white;
  white.r[AMBIENT_R] = 0.1;
  white.g[AMBIENT_R] = 0.1;
  white.b[AMBIENT_R] = 0.1;

  white.r[DIFFUSE_R] = 0.5;
  white.g[DIFFUSE_R] = 0.5;
  white.b[DIFFUSE_R] = 0.5;

  white.r[SPECULAR_R] = 0.5;
  white.g[SPECULAR_R] = 0.5;
  white.b[SPECULAR_R] = 0.5;

  //constants are a pointer in symtab, using one here for consistency
  struct constants *reflect;
  reflect = &white;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen(t);
  clear_zbuffer(zb);
  g.red = 100;
  g.green = 100;
  g.blue = 100;
  
  double x,y,z,r0,r1, x1,y1,z1, angle;
  struct matrix *top;

  print_symtab();
  for (i = 0; i < lastop; i++)
  {

    switch (op[i].opcode)
    {
    case LIGHT:
      light[COLOR][RED] = op[i].op.light.c[0];
      light[COLOR][GREEN] = op[i].op.light.c[1];
      light[COLOR][BLUE] = op[i].op.light.c[2];
      break;
    case AMBIENT:
      ambient.red = op[i].op.ambient.c[0];
      ambient.green = op[i].op.ambient.c[1];
      ambient.blue = op[i].op.ambient.c[2];
      break;

    case CONSTANTS:
      break;
    case SAVE_COORDS:
      break;
    case CAMERA:
      break;
    case SPHERE:
      // printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
      //        op[i].op.sphere.d[0], op[i].op.sphere.d[1],
      //        op[i].op.sphere.d[2],
      //        op[i].op.sphere.r);
      // if (op[i].op.sphere.constants != NULL)
      // {
      //   printf("\tconstants: %s", op[i].op.sphere.constants->name);
      // }
      // if (op[i].op.sphere.cs != NULL)
      // {
      //   printf("\tcs: %s", op[i].op.sphere.cs->name);
      // }

      x = op[i].op.sphere.d[0];
      y = op[i].op.sphere.d[1];
      z = op[i].op.sphere.d[2];
      r0 = op[i].op.sphere.r;

      if (op[i].op.sphere.constants != NULL) {
        reflect = op[i].op.sphere.constants->s.c;
      }

      add_sphere(tmp, x, y, z, r0, step_3d);
      matrix_mult(peek(systems), tmp);
      draw_polygons(tmp, t, zb, view, light, ambient, reflect);
      tmp->lastcol = 0;

      reflect = &white;

      break;
    case TORUS:
      x = op[i].op.torus.d[0];
      y = op[i].op.torus.d[1];
      z = op[i].op.torus.d[2];
      r0 = op[i].op.torus.r0;
      r1 = op[i].op.torus.r1;

      if (op[i].op.torus.constants != NULL) {
        reflect = op[i].op.torus.constants->s.c;
      }

      add_torus(tmp, x, y, z, r0, r1, step_3d);
      matrix_mult(peek(systems), tmp);
      draw_polygons(tmp, t, zb, view, light, ambient, reflect);
      tmp->lastcol = 0;

      //printf("coL: %lf %lf %lf \n", reflect->r[1], reflect->g[1], reflect->b[1]);

      reflect = &white;

      break;
    case BOX:
      // printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
      //        op[i].op.box.d0[0], op[i].op.box.d0[1],
      //        op[i].op.box.d0[2],
      //        op[i].op.box.d1[0], op[i].op.box.d1[1],
      //        op[i].op.box.d1[2]);
      // if (op[i].op.box.constants != NULL)
      // {
      //   printf("\tconstants: %s", op[i].op.box.constants->name);
      // }
      // if (op[i].op.box.cs != NULL)
      // {
      //   printf("\tcs: %s", op[i].op.box.cs->name);
      // }

      x = op[i].op.box.d0[0];
      y = op[i].op.box.d0[1];
      z = op[i].op.box.d0[2];
      x1 = op[i].op.box.d1[0];
      y1 = op[i].op.box.d1[1];
      z1 = op[i].op.box.d1[2];

      //printf("rL: %lf %lf %lf \n", reflect->r[1], reflect->g[1], reflect->b[1]);

      if (op[i].op.box.constants != NULL) {
        reflect = op[i].op.box.constants->s.c;
      }

      //printf("coL: %lf %lf %lf \n", reflect->r[1], reflect->g[1], reflect->b[1]);

      add_box(tmp, x, y, z, x1, y1, z1);
      matrix_mult(peek(systems), tmp);
      draw_polygons(tmp, t, zb, view, light, ambient, reflect);
      tmp->lastcol = 0;

      reflect = &white;

      break;
    case LINE:
      // printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
      //        op[i].op.line.p0[0], op[i].op.line.p0[1],
      //        op[i].op.line.p0[1],
      //        op[i].op.line.p1[0], op[i].op.line.p1[1],
      //        op[i].op.line.p1[1]);
      // if (op[i].op.line.constants != NULL)
      // {
      //   printf("\n\tConstants: %s", op[i].op.line.constants->name);
      // }
      // if (op[i].op.line.cs0 != NULL)
      // {
      //   printf("\n\tCS0: %s", op[i].op.line.cs0->name);
      // }
      // if (op[i].op.line.cs1 != NULL)
      // {
      //   printf("\n\tCS1: %s", op[i].op.line.cs1->name);
      // }

      x = op[i].op.line.p0[0];
      y = op[i].op.line.p0[1];
      z = op[i].op.line.p0[2];
      x1 = op[i].op.line.p1[0];
      y1 = op[i].op.line.p1[1];
      z1 = op[i].op.line.p1[2];

      if (op[i].op.torus.constants != NULL) {
        reflect = op[i].op.box.constants->s.c;
      }

      add_edge(tmp, x, y, z, x1, y1, z1);
      matrix_mult(peek(systems), tmp);
      draw_lines(tmp,t,zb,g);
      //draw_polygons(tmp, t, zb, view, light, ambient, reflect);
      tmp->lastcol = 0;

      reflect = &white;

      break;
    case MESH:
      break;
    case SET:
      break;
    case MOVE:
      // printf("Move: %6.2f %6.2f %6.2f",
      //        op[i].op.move.d[0], op[i].op.move.d[1],
      //        op[i].op.move.d[2]);
      // if (op[i].op.move.p != NULL)
      // {
      //   printf("\tknob: %s", op[i].op.move.p->name);
      // }
      x = op[i].op.move.d[0];
      y = op[i].op.move.d[1];
      z = op[i].op.move.d[2];
      top = make_translate(x, y, z);
      matrix_mult(peek(systems),top);
      copy_matrix(top, peek(systems));
      free_matrix(top);
      break;
    case SCALE:
      // printf("Scale: %6.2f %6.2f %6.2f",
      //        op[i].op.scale.d[0], op[i].op.scale.d[1],
      //        op[i].op.scale.d[2]);
      // if (op[i].op.scale.p != NULL)
      // {
      //   printf("\tknob: %s", op[i].op.scale.p->name);
      // }
      x = op[i].op.scale.d[0];
      y = op[i].op.scale.d[1];
      z = op[i].op.scale.d[2];
      //printf("scale: %lf %lf %lf\n", x, y, z);
      top = make_scale(x, y, z);
      matrix_mult(peek(systems),top);
      copy_matrix(top, peek(systems));
      free_matrix(top);
      break;
    case ROTATE:
      // printf("Rotate: axis: %6.2f degrees: %6.2f",
      //        op[i].op.rotate.axis,
      //        op[i].op.rotate.degrees);
      // if (op[i].op.rotate.p != NULL)
      // {
      //   printf("\tknob: %s", op[i].op.rotate.p->name);
      // }
      angle = op[i].op.rotate.degrees * (M_PI / 180);
      if (op[i].op.rotate.axis == 0) {
        top = make_rotX(angle);
      } else if (op[i].op.rotate.axis == 1) {
        top = make_rotY(angle);
      } else if (op[i].op.rotate.axis == 2) {
        top = make_rotZ(angle);
      }
      matrix_mult(peek(systems), top);
      copy_matrix(top, peek(systems));
      free_matrix(top);
      break;
    case BASENAME:
      break;
    case SAVE_KNOBS:
      break;
    case TWEEN:
      break;
    case FRAMES:
      break;
    case VARY:
      break;
    case PUSH:
      push(systems);
      break;
    case POP:
      pop(systems);
      break;
    case GENERATE_RAYFILES:
      break;
    case SAVE:
      save_extension(t, op[i].op.save.p->name);
      break;
    case SHADING:
      break;
    case SETKNOBS:
      break;
    case FOCAL:
      break;
    case DISPLAY:
      display(t);
      break;
    }

  }
}
