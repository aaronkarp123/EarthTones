# EarthTones

This repository stores the completed VCV rack plug-ins developed by Aaron Karp, under the name *EarthTones*.

## PolarCV

This is a module that generates control voltages that correspond to a point traveling along various polar equations.

<img width="270" alt="image" src="https://user-images.githubusercontent.com/8389851/204151257-8c59da77-bf39-4a3f-b88b-229fff34f771.png">


The red dot indicates the current position, which travels along the equation lines.

The left-most control knob corresponds to the speed at which the dot travels (the **ΔΘ**). That can be adjusted by rotating the knob, or be controlled directly from a control voltage using the input to its left. The **mult** button to the upper left of the speed dial modifies the speed by a fixed amount. This is either 0.5x, 1.0x, or 2.0x. 

The current equation (along with the speed multiplier) can be seen in the top bar. There are a total of four different equations, which can be switched using the → and ← buttons. The equations all have two constants, **A** and **B**, which can be changed via the **A** and **B** knobs. Changing these values can create very different results, so I would encourage exploring different combinations of equations and constants.

There are three outputs:
- **r** is the radius of the current position (in polar coordinates)
- **x** is the horizontal component of the current position (in cartesian coordinates)
- **y** is the vertical component of the current position (in cartesian coordinates)

Below are some examples of different equations that can be created with this module:

<img width="270" alt="image" src="https://user-images.githubusercontent.com/8389851/204151930-b8856f3d-0dcd-441d-8fc9-92b1d04ba9d0.png"> <img width="268" alt="image" src="https://user-images.githubusercontent.com/8389851/204151941-2fe0ab91-527d-49a9-ac8f-1d2de3815d41.png"><img width="269" alt="image" src="https://user-images.githubusercontent.com/8389851/204152072-3aac5745-5a17-4893-b1d4-6c5d5f0e7b3c.png">


<img width="269" alt="image" src="https://user-images.githubusercontent.com/8389851/204152234-4f905a29-6a69-46b5-9381-7aa3909da2db.png"><img width="270" alt="image" src="https://user-images.githubusercontent.com/8389851/204151975-42474c7d-f1f8-46e3-bd65-63ab668b3cf8.png"><img width="269" alt="image" src="https://user-images.githubusercontent.com/8389851/204151959-08f58823-90d8-45f9-8e2d-de8f462fa0c9.png">
