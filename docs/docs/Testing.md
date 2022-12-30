## Testing Option for VSCode

You can test your functions directly on the device. 

## Structure

All tests are under directory "test" in the project and not compiled with default build option of platformio. The main function is in file `test_suite_controlflow.cpp`. In method `app_main()` you can add your own tests. 

<img width="400" alt="image" src="https://user-images.githubusercontent.com/412645/209811778-7efe3b83-8954-4d3b-afa3-d3718fcd9058.png">

## Include my my own test

In method `app_main()` of `test_suite_controlflow.cpp` you can add your own tests. Include your test-file in the top like

```#include "components/jomjol-flowcontroll/test_flow_postrocess_helper.cpp"```

components is a subfolder of tests here. Not the components directory of root source.

In the bottom add your test function.

```RUN_TEST(testNegative);```

Your test function should have a `TEST_ASSERT_EQUAL_*`. For more information look at [unity-testing](https://docs.platformio.org/en/latest/advanced/unit-testing/frameworks/unity.html). 

## Run tests

You will need a testing device. best with usb adapter. Before you upload your tests you will need to setup the device with initial setup procedure described in [[Installation]]

<img width="300" alt="image" src="https://user-images.githubusercontent.com/412645/209813215-e0ea7405-6ff4-48d0-8dab-97bfab6962af.png">


Now you can use Visual Studio Code or a standard console to upload the test code. In VS Code (tab platformio) open _Advanced_ and select _Test_.

<img width="467" alt="image" src="https://user-images.githubusercontent.com/412645/209813917-ea7fca50-2553-4acf-a8af-ecdac84a01ea.png">


Alternativ you can run it in console/terminal with `platformio test --environment esp32cam`.

In my environment the serial terminal not opens. I have to do it for myself. You will see much logging. If any test fails it logs it out. Else it logs all test passed in the end.


## Troubleshooting

If you test very much cases in one function, the device runs in stackoverflow and an endless boot. Reduce the count of test cases or split the test function in multiple functions.
