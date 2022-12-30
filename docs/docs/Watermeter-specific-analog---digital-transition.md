# Understanding the problem

At first, for the most watermeters the default configuration should be work. But the digit, especially the last digit differs in some devices.

## "Normal" transition

In most cases, the transition of the last digit starts when the analogue pointer is > 9. 

Often the last digit "hangs" a bit on this devices and comes not over zero. So it is not easy to see which digit is correct. In the first example 4 or still 3? (3 is correct).

<img width="122" alt="image" src="https://user-images.githubusercontent.com/412645/209808192-5ff67e9f-ea7c-4d82-a8e4-54b3643c7e24.png">
<img width="122" alt="image" src="https://user-images.githubusercontent.com/412645/209808306-359cce2e-ec84-4390-82d1-6747e1ec056c.png">


## Early transition

Some units start the transition very early or run with the analogue pointer. In the third example, is it a 3 or a 2?

<img width="122" alt="image" src="https://user-images.githubusercontent.com/412645/209807685-658fb9bb-648a-4779-bc30-805eadc12083.png">
<img width="122" alt="image" src="https://user-images.githubusercontent.com/412645/209808972-448bb6d0-7b7e-4030-abb2-9c966ceffc4a.png">
<img width="122" alt="image" src="https://user-images.githubusercontent.com/412645/209809116-d4acc5f2-ab5c-4304-9559-598b1dfc59c2.png">


## Inaccuracies in image recognition

The models for image recognition are good, but have inaccuracies in the range +/- 0.2. In order to obtain as many correct results as possible, a treatment is carried out in the post process in the range of 9.8-0.2 for the analogue pointer, which must start differently depending on the type of counter.


## How to configure for my meter type

If you have a devices with "normal" transition you should not have any issues. On devices with "early" transition, you can set the option `AnalogDigitalTransitionStart` to a value between 6 and 8.

