// THERE IS NO WARRANTY FOR THE SOFTWARE, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR
// OTHER PARTIES PROVIDE THE SOFTWARE “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE SOFTWARE IS WITH THE CUSTOMER. SHOULD THE
// SOFTWARE PROVE DEFECTIVE, THE CUSTOMER ASSUMES THE COST OF ALL NECESSARY SERVICING, REPAIR, OR CORRECTION EXCEPT TO THE EXTENT SET OUT UNDER THE HARDWARE WARRANTY IN THESE TERMS.

//============================================================================//
/*!
 *      ___        ___       _   _                _ _            _
 *     |  _|  /\  |_  |     | | | |              (_) |        /\| |/\
 *     | |   /  \   | |_   _| |_| |__   ___  _ __ _| |_ _   _ \ ` ' / ______
 *     | |  / /\ \  | | | | | __| '_ \ / _ \| '__| | __| | | |_     _|______|
 *     | | / ____ \ | | |_| | |_| | | | (_) | |  | | |_| |_| |/ , . \
 *     | |/_/    \_\| |\__,_|\__|_| |_|\___/|_|  |_|\__|\__, |\/|_|\/
 *     |___|      |___|                                  __/ |
 *                                                      |___/
 *
 * @author  Shah Zaman Haider
 * @date    30th August 2024
 * @file    main.cpp
 * @version v1.0.0 -- CHANGELOG
 *              | v1.0.0 -> Added functionality to control drone through pc or laptop using web app
 *
 * @details This program is written in C++ language using Arduino framework for
 *          ESP32 platform in VS CODE using PlatformIO
 *
 *          This program deals with drone control*
 *
 * @note    IMPORTANT:
 *              - Select board "esp32doit-devkit-v1"
 *              - Select Framework "Arduino"
 */
//============================================================================//

///////////////////////////////////////////////////////////////////////////////////
// MAIN WEB PAGE FOR DRONE CONTROLLER
///////////////////////////////////////////////////////////////////////////////////

const char *index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Drone Control</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        #container {
            text-align: center;
            background-color: #ffffff;
            border-radius: 10px;
            padding: 40px;
            box-shadow: 0px 4px 20px rgba(0, 0, 0, 0.1);
            width: 400px;
        }
        h1 {
            color: #333333;
            font-size: 36px;
            margin-bottom: 10px;
            margin-top: 0;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
        }
        p {
            color: #666666;
            font-size: 18px;
            margin-bottom: 20px;
        }
        #arrowRepresentation {
            font-size: 100px;
            margin-top: 20px;
            color: #007bff;
            transition: opacity 0.3s ease-in-out;
        }
        #arrowRepresentation.hidden {
            opacity: 0;
        }
        .dark-green-button {
            background-color: rgb(143, 101, 4);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin-top: 20px;
            cursor: pointer;
            transition: background-color 0.3s ease, transform 0.3s ease;
        }
        input[type="number"] {
            flex: 2;
            width: 50%;
            padding: 10px;
            font-size: 16px;
            border: 1px solid #cccccc;
            border-radius: 5px;
            margin-right: 10px;
            margin-top: 15px;
        }
        label {
            flex: 1;
            font-size: 16px;
            color: #666666;
        }
        #keyValueDisplay {
            transition: opacity 0.5s ease-out;
        }
        #keyValueDisplay.fade-out {
            opacity: 0;
        }
    </style>
</head>
<body>
    <div id="container">
        <h1><span style="color: #007bff;">D</span>rone <span style="color: #007bff;">C</span>ontrol</h1>
        <p>Use arrow keys to control the drone.</p>
        <div id="arrowRepresentation" class="hidden"></div>
        <div id="keyValueDisplay"></div>
        <button class="dark-green-button" onclick="redirectToAnotherPage()">Modify the PID values</button>
        <input type="number" id="speed" name="speed">
    </div>
  <script>
    var pressedKeys = [];
    var speed = 1000;
    var minSpeed = 1000;
    var maxSpeed = 2000;
    var accelerationInterval = null;
    var decelerationInterval = null;
    var accelerationRate = 10;
    var decelerationRate = 100;
    var decelerationRate_downkey = 10;

    var upArrowSpeed = 1000;
    var currentKeySpeeds = {};

    var socket = new WebSocket('ws://' + window.location.hostname + '/ws');

    socket.onopen = function() {
        console.log('WebSocket connection opened');
    };

    socket.onclose = function() {
        console.log('WebSocket connection closed');
    };

    socket.onmessage = function(event) {
        console.log('Message from server: ' + event.data);
    };

    document.addEventListener('keydown', function(event) {
        if (isValidKey(event.keyCode) && !pressedKeys.includes(event.keyCode)) {
            event.preventDefault();
            pressedKeys.push(event.keyCode);
            if (event.keyCode === 38) {
                startAcceleration(true);
            } else if (event.keyCode === 40) {
                startDeceleration(true);
            } else {
                if (!currentKeySpeeds[event.keyCode]) {
                    currentKeySpeeds[event.keyCode] = 0;
                }
                startAcceleration(false);
            }
            var arrowSymbol = getArrowSymbol(pressedKeys);
            displayArrow(arrowSymbol);
            sendCommand(pressedKeys);
        }
    });

    document.addEventListener('keyup', function(event) {
        if (pressedKeys.includes(event.keyCode)) {
            event.preventDefault();
            pressedKeys.splice(pressedKeys.indexOf(event.keyCode), 1);
            if (event.keyCode === 38) {
                stopAcceleration();
            } else if (event.keyCode === 40) {
                stopDeceleration();
            } else {
                stopDecelerationForKey(event.keyCode);
            }
            if (pressedKeys.length === 0) {
                hideArrow();
                sendStopCommand();
            } else {
                var arrowSymbol = getArrowSymbol(pressedKeys);
                displayArrow(arrowSymbol);
                sendCommand(pressedKeys);
            }
            fadeOutKeyValueDisplay();
        }
    });

    function isValidKey(keyCode) {
        return [37, 38, 39, 40, 65, 68, 87, 83].includes(keyCode);
    }

    function startAcceleration(isUpArrow) {
    clearInterval(accelerationInterval);
    accelerationInterval = setInterval(function() {
        if (isUpArrow) {
            if (upArrowSpeed < maxSpeed) {
                upArrowSpeed += accelerationRate;
                if (upArrowSpeed > maxSpeed) {
                    upArrowSpeed = maxSpeed;
                }
                speed = upArrowSpeed;
                updateSpeedInput();
            }
        } else {
            var keyCode = pressedKeys.find(key => key !== 38 && key !== 40);
            if (keyCode !== undefined) {
                if (currentKeySpeeds[keyCode] < maxSpeed) {
                    currentKeySpeeds[keyCode] += accelerationRate;
                    if (currentKeySpeeds[keyCode] > maxSpeed) {
                        currentKeySpeeds[keyCode] = maxSpeed;
                    }
                    speed = Math.min(maxSpeed, upArrowSpeed + currentKeySpeeds[keyCode]);
                    updateSpeedInput();
                    displayKeyValue(upArrowSpeed, currentKeySpeeds[keyCode], speed);
                }
            }
        }
        sendCommand();
    }, 100);
}

function startDeceleration(isDownArrow) {
    clearInterval(decelerationInterval);
    decelerationInterval = setInterval(function() {
        if (isDownArrow) {
            if (speed > minSpeed) {
                speed -= decelerationRate_downkey;
                if (speed < minSpeed) {
                    speed = minSpeed;
                }
                upArrowSpeed = speed;
                updateSpeedInput();
            }
        } else {
            var keyCode = pressedKeys.find(key => key !== 38 && key !== 40);
            if (keyCode !== undefined) {
                if (currentKeySpeeds[keyCode] > 0) {
                    currentKeySpeeds[keyCode] -= decelerationRate;
                    if (currentKeySpeeds[keyCode] < 0) {
                        currentKeySpeeds[keyCode] = 0;
                    }
                    speed = Math.min(maxSpeed, upArrowSpeed + currentKeySpeeds[keyCode]);
                    updateSpeedInput();
                    displayKeyValue(upArrowSpeed, currentKeySpeeds[keyCode], speed);
                }
            }
        }
        sendCommand();
    }, 100);
}

    function stopAcceleration() {
        clearInterval(accelerationInterval);
    }

    function stopDeceleration() {
        clearInterval(decelerationInterval);
    }

    function getCurrentKeySpeedsSum() {
    return Object.values(currentKeySpeeds).reduce((a, b) => a + b, 0);
}


    function stopDecelerationForKey(keyCode) {
        clearInterval(decelerationInterval);
        decelerationInterval = setInterval(function() {
            if (currentKeySpeeds[keyCode] > 0) {
                currentKeySpeeds[keyCode] -= decelerationRate;
                if (currentKeySpeeds[keyCode] < 0) {
                    currentKeySpeeds[keyCode] = 0;
                }
                speed = Math.min(maxSpeed, upArrowSpeed + currentKeySpeeds[keyCode]);
                updateSpeedInput();
                displayKeyValue(upArrowSpeed, currentKeySpeeds[keyCode], speed);
            }
            sendCommand(pressedKeys);
        }, 100);
    }

    function updateSpeedInput() {
        var speedInput = document.getElementById("speed");
        speedInput.value = speed;
    }

    function getArrowSymbol(keyCodes) {
        var symbols = keyCodes.map(function(keyCode) {
            switch (keyCode) {
                case 37:
                    return "&larr;";
                case 38:
                    return "&uarr;";
                case 39:
                    return "&rarr;";
                case 40:
                    return "&darr;";
                case 65:
                    return "<span style='font-size: 40px;'>Rotate Left</span>";
                case 68:
                    return "<span style='font-size: 40px;'>Rotate Right</span>";
                case 87:
                    return "<span style='font-size: 40px;'>Forward</span>";
                case 83:
                    return "<span style='font-size: 40px;'>Backward</span>";
                default:
                    return "";
            }
        });
        return symbols.join(" ");
    }

    function displayArrow(symbol) {
        var arrowElement = document.getElementById('arrowRepresentation');
        arrowElement.innerHTML = symbol;
        arrowElement.classList.remove('hidden');
    }

    function hideArrow() {
        var arrowElement = document.getElementById('arrowRepresentation');
        arrowElement.classList.add('hidden');
    }

    function displayKeyValue(upArrowSpeed, currentKeySpeed, totalSpeed) {
        var keyValueDisplay = document.getElementById('keyValueDisplay');
        keyValueDisplay.innerHTML = `${upArrowSpeed} + ${currentKeySpeed} = ${totalSpeed}`;
    }

    function fadeOutKeyValueDisplay() {
        var keyValueDisplay = document.getElementById('keyValueDisplay');
        keyValueDisplay.classList.add('fade-out');
        setTimeout(function() {
            keyValueDisplay.classList.remove('fade-out');
            keyValueDisplay.innerHTML = '';
        }, 500);
    }

    function sendCommand() {
    var command = pressedKeys.map(function(keyCode) {
        switch (keyCode) {
            case 37:
                return "Roll Left";
            case 38:
                return "Throttle Up";
            case 39:
                return "Roll Right";
            case 40:
                return "Throttle Down";
            case 65:
                return "Yaw Left";
            case 68:
                return "Yaw Right";
            case 87:
                return "Pitch Forward";
            case 83:
                return "Pitch Backward";
            default:
                return "";
        }
    }).join(", ");
    
    var totalSpeed = Math.min(maxSpeed, upArrowSpeed + getCurrentKeySpeedsSum());
    socket.send(command + "," + totalSpeed + "," + upArrowSpeed);
}

    
    function sendStopCommand() {
        socket.send("Stop");
    }

    function getSpeed() {
    return document.getElementById("speed").value;
}

function getUpArrowSpeed() {
    return upArrowSpeed;
}

function redirectToAnotherPage() {
            
                window.open("/pid", "_self");
            }

</script>
</body>
</html>

)rawliteral";

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// WEB PAGE FOR PID VALUES SETTINGS
///////////////////////////////////////////////////////////////////////////////////

const char *index_html_pid = R"rawliteral( 
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script>
        function submitMessage() {
            alert("Saved value to ESP SPIFFS");
            setTimeout(function () { document.location.reload(false); }, 500);
        }
    </script>
    <title>Drone Tuning</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f7f7f7;
            margin: 0;
            padding: 20px;
        }

        h1 {
            font-size: 24px;
            color: #333333;
            margin-bottom: 50px;
            text-align: center;
        }
    </style>
</head>

<body>
    <form action="/get" target="hidden-form"><br>
        <h1> ESP32 Webserver for PID Gain value tuning of Quadcopter </h1>
    </form><br><br>

    <form action="/get" target="hidden-form">
        P Pitch & Roll Gain (current value %pGain%): <input type="number" step="any" name="pGain">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br>
    <form action="/get" target="hidden-form">
        I Pitch & Roll Gain (current value %iGain%): <input type="number" step="any" name="iGain">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br>
    <form action="/get" target="hidden-form">
        D Pitch & Roll Gain (current value %dGain%): <input type="number" step="any" name="dGain">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br>
    <form action="/get" target="hidden-form">
        P Yaw Gain (current value %pYaw%): <input type="number" step="any" name="pYaw">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br>
    <form action="/get" target="hidden-form">
        I Yaw Gain (current value %iYaw%): <input type="number" step="any" name="iYaw">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br>
    <form action="/get" target="hidden-form">
        D Yaw Gain (current value %dYaw%): <input type="number" step="any" name="dYaw">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br><br>
    <form action="/get" target="hidden-form">
        Time cycle (current value %tc%): <input type="number" step="any" name="tc">
        <input type="submit" value="Submit" onclick="submitMessage()">
    </form><br><br>

    <iframe style="display:none" name="hidden-form"></iframe>


</body>

</html>)rawliteral";

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <ESP32Servo.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char *ssid = "XYZ123";
const char *password = "xyz12345";

Servo mot1;
Servo mot2;
Servo mot3;
Servo mot4;

// ESP32 GPIO Pins to connect these with ESC's signal wires
const int mot1_pin = 19;
const int mot2_pin = 23;
const int mot3_pin = 26;
const int mot4_pin = 27;
const int LED = 15;
int speed;

int MotorInput1, MotorInput2, MotorInput3, MotorInput4;

//************************************************************************* */
// Some constants and variables for PID tuning
//************************************************************************* */

const char *PARAM_P_GAIN = "pGain"; // For Pitch & Roll
const char *PARAM_I_GAIN = "iGain";
const char *PARAM_D_GAIN = "dGain";
const char *PARAM_P_YAW = "pYaw"; // For Yaw
const char *PARAM_I_YAW = "iYaw";
const char *PARAM_D_YAW = "dYaw";
const char *PARAM_TIME_CYCLE = "tc"; // Computation time cycle

volatile uint32_t current_time;

volatile float RatePitch, RateRoll, RateYaw;
volatile float RateCalibrationPitch, RateCalibrationRoll, RateCalibrationYaw;
int RateCalibrationNumber;

float DesiredRateRoll, DesiredRatePitch, DesiredRateYaw;
float ErrorRateRoll, ErrorRatePitch, ErrorRateYaw;
float InputRoll, InputThrottle, InputPitch, InputYaw;
float PrevErrorRateRoll, PrevErrorRatePitch, PrevErrorRateYaw;
float PrevItermRateRoll, PrevItermRatePitch, PrevItermRateYaw;
float PIDReturn[] = {0, 0, 0};

// float AccX, AccY, AccZ;
// float AngleRoll, AnglePitch;
// float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=2*2;
// float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=2*2;
// float Kalman1DOutput[]={0,0};

float PRateRoll = 0.75; // For outdoor flights, keep this gain to 0.75 and for indoor flights keep the gain to be 0.6
float IRateRoll = 0.012;
float DRateRoll = 0.0085;

float PRatePitch = PRateRoll;
float IRatePitch = IRateRoll;
float DRatePitch = DRateRoll;

float PRateYaw = 4.2;
float IRateYaw = 2.8;
float DRateYaw = 0;

uint32_t LoopTimer;
float t = 0.006; // time cycle

// Kalman filters for angle mode
volatile float AccX, AccY, AccZ;
volatile float AngleRoll, AnglePitch;
volatile float KalmanAngleRoll = 0, KalmanUncertaintyAngleRoll = 2 * 2;
volatile float KalmanAnglePitch = 0, KalmanUncertaintyAnglePitch = 2 * 2;
volatile float Kalman1DOutput[] = {0, 0};
volatile float DesiredAngleRoll, DesiredAnglePitch;
volatile float ErrorAngleRoll, ErrorAnglePitch;
volatile float PrevErrorAngleRoll, PrevErrorAnglePitch;
volatile float PrevItermAngleRoll, PrevItermAnglePitch;
float PAngleRoll = 2;
float PAnglePitch = PAngleRoll;
float IAngleRoll = 0;
float IAnglePitch = IAngleRoll;
float DAngleRoll = 0;
float DAnglePitch = DAngleRoll;

void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement)
{
    KalmanState = KalmanState + (t * KalmanInput);
    KalmanUncertainty = KalmanUncertainty + (t * t * 4 * 4);                    // here 4 is the vairnece of IMU i.e 4 deg/s
    float KalmanGain = KalmanUncertainty * 1 / (1 * KalmanUncertainty + 3 * 3); // std deviation of error is 3 deg
    KalmanState = KalmanState + KalmanGain * (KalmanMeasurement - KalmanState);
    KalmanUncertainty = (1 - KalmanGain) * KalmanUncertainty;
    Kalman1DOutput[0] = KalmanState;
    Kalman1DOutput[1] = KalmanUncertainty;
}

//************************************************************************* */
// Read and Write functions to store tuned PID values in ESP32 Memory
//************************************************************************* */

String readFile(fs::FS &fs, const char *path);

// File reading function
String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);
    File file = fs.open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial.println("- empty file or failed to open file");
        return String();
        ;
    }
    Serial.println("- read from file:");
    String fileContent;
    while (file.available())
    {
        fileContent += String((char)file.read());
    }
    file.close();
    Serial.println(fileContent);
    return fileContent;
}

// File writing function
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\r\n", path);
    File file = fs.open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}

// If Webserver Not found
void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "It seems you've traveled to a page that doesn't exist in this timeline. Maybe try a different year—or just go back to our homepage.");
}

//************************************************************************* */
// Functions for PID tuning
//************************************************************************* */

// Replaces placeholder with stored values
String processor(const String &var)
{
    // Serial.println(var);
    if (var == "pGain")
    {
        return readFile(SPIFFS, "/pid/pGain.txt");
    }
    else if (var == "iGain")
    {
        return readFile(SPIFFS, "/pid/iGain.txt");
    }
    else if (var == "dGain")
    {
        return readFile(SPIFFS, "/pid/dGain.txt");
    }
    else if (var == "pYaw")
    {
        return readFile(SPIFFS, "/pid/pYaw.txt");
    }
    else if (var == "dYaw")
    {
        return readFile(SPIFFS, "/pid/dYaw.txt");
    }
    else if (var == "iYaw")
    {
        return readFile(SPIFFS, "/pid/iYaw.txt");
    }
    else if (var == "tc")
    {
        return readFile(SPIFFS, "/pid/tc.txt");
    }
}

// Gyroscope sensor code
void gyro_signals(void)
{
    Wire.beginTransmission(0x68);
    Wire.write(0x1A);
    Wire.write(0x05);
    Wire.endTransmission();
    Wire.beginTransmission(0x68);
    Wire.write(0x1C);
    Wire.write(0x10);
    Wire.endTransmission();
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);
    int16_t AccXLSB = Wire.read() << 8 | Wire.read();
    int16_t AccYLSB = Wire.read() << 8 | Wire.read();
    int16_t AccZLSB = Wire.read() << 8 | Wire.read();
    Wire.beginTransmission(0x68);
    Wire.write(0x1B);
    Wire.write(0x8);
    Wire.endTransmission();
    Wire.beginTransmission(0x68);
    Wire.write(0x43);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 6);
    int16_t GyroX = Wire.read() << 8 | Wire.read();
    int16_t GyroY = Wire.read() << 8 | Wire.read();
    int16_t GyroZ = Wire.read() << 8 | Wire.read();
    RateRoll = (float)GyroX / 65.5;
    RatePitch = (float)GyroY / 65.5;
    RateYaw = (float)GyroZ / 65.5;
    AccX = (float)AccXLSB / 4096;
    AccY = (float)AccYLSB / 4096;
    AccZ = (float)AccZLSB / 4096;
    AccZ = AccZ - 0.26; // calibration offset
    AngleRoll = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 1 / (3.142 / 180);
    AnglePitch = -atan(AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 1 / (3.142 / 180);
}

void pid_equation(float Error, float P, float I, float D, float PrevError, float PrevIterm)
{
    float Pterm = P * Error;
    float Iterm = PrevIterm + (I * (Error + PrevError) * (t / 2));
    if (Iterm > 400)
    {
        Iterm = 400;
    }
    else if (Iterm < -400)
    {
        Iterm = -400;
    }
    float Dterm = D * ((Error - PrevError) / t);
    float PIDOutput = Pterm + Iterm + Dterm;
    if (PIDOutput > 400)
    {
        PIDOutput = 400;
    }
    else if (PIDOutput < -400)
    {
        PIDOutput = -400;
    }
    PIDReturn[0] = PIDOutput;
    PIDReturn[1] = Error;
    PIDReturn[2] = Iterm;
}

void reset_pid(void)
{
    PrevErrorRateRoll = 0;
    PrevErrorRatePitch = 0;
    PrevErrorRateYaw = 0;
    PrevItermRateRoll = 0;
    PrevItermRatePitch = 0;
    PrevItermRateYaw = 0;
    PrevErrorAngleRoll = 0;
    PrevErrorAnglePitch = 0;
    PrevItermAngleRoll = 0;
    PrevItermAnglePitch = 0;
}

//******************************************************************************** */
// Functions to recive pressed keys from controller(webserver) and process it
//******************************************************************************** */

void process_command(String command, String speedStr, String upArrowSpeedStr);

// Function to receive commands from Webserver using Websocket technology and filter it
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        String message = (char *)data;

        // Split the message to extract command, speed, and upArrowSpeed
        int firstCommaIndex = message.indexOf(',');
        int secondCommaIndex = message.indexOf(',', firstCommaIndex + 1);
        if (firstCommaIndex != -1 && secondCommaIndex != -1)
        {
            String command = message.substring(0, firstCommaIndex);
            String speedStr = message.substring(firstCommaIndex + 1, secondCommaIndex);
            String upArrowSpeedStr = message.substring(secondCommaIndex + 1);

            // Process the command

            process_command(command, speedStr, upArrowSpeedStr);
        }
    }
}

// Process and perform action
void process_command(String command, String speedStr, String upArrowSpeedStr)
{

    speed = speedStr.toInt();
    int upArrowSpeed = upArrowSpeedStr.toInt();

    MotorInput1 = speed; // front right - counter clockwise
    MotorInput2 = speed; // rear left   - counter clockwise
    MotorInput3 = speed; // front left  -  clockwise
    MotorInput4 = speed; // rear right -   clockwise

    if (command == "Yaw Left")
    {

        // rotate left (yaw left)

        // Decrease the speed of the clockwise (CW) motors (MotorInput3 and MotorInput4).
        // Increase the speed of the counterclockwise (CCW) motors (MotorInput1 and MotorInput2).

        MotorInput3 = upArrowSpeed;
        MotorInput4 = upArrowSpeed;

        Serial.print("Rotate Left: ");
    }
    else if (command == "Yaw Right")
    {

        // rotate right (yaw right)

        // Increase the speed of the clockwise (CW) motors (MotorInput3 and MotorInput4).
        // Decrease the speed of the counterclockwise (CCW) motors (MotorInput1 and MotorInput2).

        MotorInput1 = upArrowSpeed;
        MotorInput2 = upArrowSpeed;

        Serial.print("Rotate Right: ");
    }
    else if (command == "Roll Left")
    {

        // left Arrow (roll left)
        // Decrease roll input to make the drone tilt left

        MotorInput2 = upArrowSpeed;
        MotorInput3 = upArrowSpeed;

        Serial.print("Left: ");
    }
    else if (command == "Roll Right")
    {

        // right Arrow (roll right)
        // Increase roll input to make the drone tilt right

        MotorInput1 = upArrowSpeed;
        MotorInput4 = upArrowSpeed;

        Serial.print("Right: ");
    }
    else if (command == "Throttle Up")
    {
        MotorInput1 = upArrowSpeed;
        MotorInput2 = upArrowSpeed;
        MotorInput3 = upArrowSpeed;
        MotorInput4 = upArrowSpeed;
        // Arrow up
        // Increase throttle input to make the drone up

        Serial.print("Up: ");
    }
    else if (command == "Throttle Down")
    {

        Serial.print("Down: ");
        // down
        // Decrease throttle input to make the drone down
    }
    else if (command == "Pitch Forward")
    {

        // forward (pitch forward) "w"
        // Decrease pitch input to make the drone move forward

        MotorInput1 = upArrowSpeed;
        MotorInput3 = upArrowSpeed;

        Serial.print("Forward: ");
    }
    else if (command == "Pitch Backward")
    {

        // backward (pitch backward)
        // Increase pitch input to make the drone move backward

        MotorInput2 = upArrowSpeed;
        MotorInput4 = upArrowSpeed;

        Serial.print("Backward: ");
    }

    mot1.writeMicroseconds(MotorInput1);
    mot2.writeMicroseconds(MotorInput2);
    mot3.writeMicroseconds(MotorInput3);
    mot4.writeMicroseconds(MotorInput4);

    Serial.print(MotorInput1);
    Serial.print("  ");
    Serial.print(MotorInput2);
    Serial.print("  ");
    Serial.print(MotorInput3);
    Serial.print("  ");
    Serial.print(MotorInput4);
    Serial.println("  ");
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        handleWebSocketMessage(arg, data, len);
    }
}

//************************************************************************* */
// SETUP FUNCTION
//************************************************************************* */

void setup(void)
{

    Serial.begin(115200);

    // Initialize SPIFFS
#ifdef ESP32
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
#else
    if (!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
#endif

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

    // Start WebSocket server
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Serve HTML content
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", index_html); });

    server.on("/pid", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html_pid, processor); });

    // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                String inputMessage;

                // GET P Gain value on <ESP_IP>/get?pGain=<inputMessage>
                if (request->hasParam(PARAM_P_GAIN))
                {
                  inputMessage = request->getParam(PARAM_P_GAIN)->value();
                  writeFile(SPIFFS, "/pid/pGain.txt", inputMessage.c_str());
                }
                // GET I Gain value on <ESP_IP>/get?iGain=<inputMessage>
                else if (request->hasParam(PARAM_I_GAIN))
                {
                  inputMessage = request->getParam(PARAM_I_GAIN)->value();
                  writeFile(SPIFFS, "/pid/iGain.txt", inputMessage.c_str());
                }
                // GET D Gain value on <ESP_IP>/get?dGain=<inputMessage>
                else if (request->hasParam(PARAM_D_GAIN))
                {
                  inputMessage = request->getParam(PARAM_D_GAIN)->value();
                  writeFile(SPIFFS, "/pid/dGain.txt", inputMessage.c_str());
                }
                else if (request->hasParam(PARAM_P_YAW))
                {
                  inputMessage = request->getParam(PARAM_P_YAW)->value();
                  writeFile(SPIFFS, "/pid/pYaw.txt", inputMessage.c_str());
                }
                else if (request->hasParam(PARAM_I_YAW))
                {
                  inputMessage = request->getParam(PARAM_I_YAW)->value();
                  writeFile(SPIFFS, "/pid/iYaw.txt", inputMessage.c_str());
                }
                else if (request->hasParam(PARAM_D_YAW))
                {
                  inputMessage = request->getParam(PARAM_D_YAW)->value();
                  writeFile(SPIFFS, "/pid/dYaw.txt", inputMessage.c_str());
                }
                else if (request->hasParam(PARAM_TIME_CYCLE))
                {
                  inputMessage = request->getParam(PARAM_TIME_CYCLE)->value();
                  writeFile(SPIFFS, "/pid/tc.txt", inputMessage.c_str());
                }
                else
                {
                  inputMessage = "No message sent";
                }
                Serial.println(inputMessage);
                request->send(200, "text/text", inputMessage); });

    server.begin();
    server.onNotFound(notFound);

    Wire.setClock(400000);
    Wire.begin();
    delay(250);
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    // Setup intial and final speed of BLDC motors
    mot1.attach(mot1_pin, 1000, 2000);
    mot2.attach(mot2_pin, 1000, 2000);
    mot3.attach(mot3_pin, 1000, 2000);
    mot4.attach(mot4_pin, 1000, 2000);

    // to stop esc from beeping when connect to power
    mot1.write(0);
    mot2.write(0);
    mot3.write(0);
    mot4.write(0);

    // LED INDICATION BEFORE CALLIBRATION OF SENSORS
    int led_time = 100;
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    delay(led_time);
    digitalWrite(LED, HIGH);
    delay(led_time);
    digitalWrite(LED, LOW);
    delay(led_time);
    digitalWrite(LED, HIGH);
    delay(led_time);
    digitalWrite(LED, LOW);
    delay(led_time);
    digitalWrite(LED, HIGH);
    delay(led_time);
    digitalWrite(LED, LOW);
    delay(led_time);
    digitalWrite(LED, HIGH);
    delay(led_time);
    digitalWrite(LED, LOW);
    delay(led_time);

    digitalWrite(LED, LOW);
    digitalWrite(LED, HIGH);
    delay(2000);
    digitalWrite(LED, LOW);
    delay(2000);

    for (RateCalibrationNumber = 0; RateCalibrationNumber < 4000; RateCalibrationNumber++)
    {
        gyro_signals();
        RateCalibrationRoll += RateRoll;
        RateCalibrationPitch += RatePitch;
        RateCalibrationYaw += RateYaw;
        delay(1);
    }
    RateCalibrationRoll /= 4000;
    RateCalibrationPitch /= 4000;
    RateCalibrationYaw /= 4000;
    // Gyro Calibrated Values
    //  Serial.print("Gyro Calib: ");
    //  Serial.print(RateCalibrationRoll);
    //  Serial.print("  ");
    //  Serial.print(RateCalibrationPitch);
    //  Serial.print("  ");
    //  Serial.print(RateCalibrationYaw);
    //  Serial.print(" -- ");

    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);

    // CALLIBRATION DONE

    LoopTimer = micros();
}

void loop()
{
    ws.cleanupClients();

    // ACCESS pid content

    PRateRoll = readFile(SPIFFS, "/pid/pGain.txt").toFloat();
    IRateRoll = readFile(SPIFFS, "/pid/iGain.txt").toFloat();
    DRateRoll = readFile(SPIFFS, "/pid/dGain.txt").toFloat();

    PRateYaw = readFile(SPIFFS, "/pid/pYaw.txt").toFloat();
    IRateYaw = readFile(SPIFFS, "/pid/iYaw.txt").toFloat();
    DRateYaw = readFile(SPIFFS, "/pid/dYaw.txt").toFloat();

    t = readFile(SPIFFS, "/pid/tc.txt").toFloat();

    // PID stuff
    gyro_signals();
    RateRoll -= RateCalibrationRoll;
    RatePitch -= RateCalibrationPitch;
    RateYaw -= RateCalibrationYaw;

    kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
    KalmanAngleRoll = Kalman1DOutput[0];
    KalmanUncertaintyAngleRoll = Kalman1DOutput[1];
    kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
    KalmanAnglePitch = Kalman1DOutput[0];
    KalmanUncertaintyAnglePitch = Kalman1DOutput[1];

    DesiredAngleRoll = 0.1 * (speed - 1500);
    DesiredAnglePitch = 0.1 * (speed - 1500);
    InputThrottle = speed;
    DesiredRateYaw = 0.15 * (speed - 1500);

    ErrorAngleRoll = DesiredAngleRoll - KalmanAngleRoll;
    ErrorAnglePitch = DesiredAnglePitch - KalmanAnglePitch;

    pid_equation(ErrorAngleRoll, PAngleRoll, IAngleRoll, DAngleRoll, PrevErrorAngleRoll, PrevItermAngleRoll);
    DesiredRateRoll = PIDReturn[0];
    PrevErrorAngleRoll = PIDReturn[1];
    PrevItermAngleRoll = PIDReturn[2];

    pid_equation(ErrorAnglePitch, PAnglePitch, IAnglePitch, DAnglePitch, PrevErrorAnglePitch, PrevItermAnglePitch);
    DesiredRatePitch = PIDReturn[0];
    PrevErrorAnglePitch = PIDReturn[1];
    PrevItermAnglePitch = PIDReturn[2];

    ErrorRateRoll = DesiredRateRoll - RateRoll;
    ErrorRatePitch = DesiredRatePitch - RatePitch;
    ErrorRateYaw = DesiredRateYaw - RateYaw;

    pid_equation(ErrorRateRoll, PRateRoll, IRateRoll, DRateRoll, PrevErrorRateRoll, PrevItermRateRoll);
    InputRoll = PIDReturn[0];
    PrevErrorRateRoll = PIDReturn[1];
    PrevItermRateRoll = PIDReturn[2];

    pid_equation(ErrorRatePitch, PRatePitch, IRatePitch, DRatePitch, PrevErrorRatePitch, PrevItermRatePitch);
    InputPitch = PIDReturn[0];
    PrevErrorRatePitch = PIDReturn[1];
    PrevItermRatePitch = PIDReturn[2];

    pid_equation(ErrorRateYaw, PRateYaw, IRateYaw, DRateYaw, PrevErrorRateYaw, PrevItermRateYaw);
    InputYaw = PIDReturn[0];
    PrevErrorRateYaw = PIDReturn[1];
    PrevItermRateYaw = PIDReturn[2];

    // If motor speeds get too high then these lines of code changes the speed to safe zone
    if (InputThrottle > 1800)
    {
        InputThrottle = 1800;
    }

    MotorInput1 = (InputThrottle - InputRoll - InputPitch - InputYaw); // front right - counter clockwise
    MotorInput2 = (InputThrottle - InputRoll + InputPitch + InputYaw); // rear right - clockwise
    MotorInput3 = (InputThrottle + InputRoll + InputPitch - InputYaw); // rear left  - counter clockwise
    MotorInput4 = (InputThrottle + InputRoll - InputPitch + InputYaw); // front left - clockwise

    if (MotorInput1 > 2000)
    {
        MotorInput1 = 1999;
    }

    if (MotorInput2 > 2000)
    {
        MotorInput2 = 1999;
    }

    if (MotorInput3 > 2000)
    {
        MotorInput3 = 1999;
    }

    if (MotorInput4 > 2000)
    {
        MotorInput4 = 1999;
    }

    // IF Speed is too low these commands turn off the motors to prevent battery discharge
    int ThrottleIdle = 1150;
    if (MotorInput1 < ThrottleIdle)
    {
        MotorInput1 = ThrottleIdle;
    }
    if (MotorInput2 < ThrottleIdle)
    {
        MotorInput2 = ThrottleIdle;
    }
    if (MotorInput3 < ThrottleIdle)
    {
        MotorInput3 = ThrottleIdle;
    }
    if (MotorInput4 < ThrottleIdle)
    {
        MotorInput4 = ThrottleIdle;
    }

    int ThrottleCutOff = 1000;
    if (speed < 1050)
    {
        MotorInput1 = ThrottleCutOff;
        MotorInput2 = ThrottleCutOff;
        MotorInput3 = ThrottleCutOff;
        MotorInput4 = ThrottleCutOff;
        reset_pid();
    }

    // voltage= (analogRead(36)/4096)*12.46*(35.9/36);
    // if(voltage<11.1)
    // {

    // }

    // Motor PWMs in us
    // Serial.print("Motorinput1");
    // Serial.print(MotorInput1);
    // Serial.print("  ");
    // Serial.print("Motorinput2");
    // Serial.print(MotorInput2);
    // Serial.print("  ");
    // Serial.print("Motorinput3");
    // Serial.print(MotorInput3);
    // Serial.print("  ");
    // Serial.print("Motorinput4");
    // Serial.print(MotorInput4);
    // Serial.print(" -- ");

    // //Reciever translated rates
    // Serial.print(DesiredRateRoll);
    // Serial.print("  ");
    // Serial.print(DesiredRatePitch);
    // Serial.print("  ");
    // Serial.print(DesiredRateYaw);
    // Serial.print(" -- ");

    // //Gyro Rates
     Serial.print("Gyro rates:");
     Serial.print(RateRoll);
     Serial.print("  ");
     Serial.print(RatePitch);
     Serial.print("  ");
     Serial.print(RateYaw);
     Serial.println(" \n------------------------------------------- \n");

    // PID outputs
     Serial.print("PID O/P: ");
     Serial.print(InputPitch);
     Serial.print("  ");
     Serial.print(InputRoll);
     Serial.print("  ");
     Serial.print(InputYaw);
     Serial.println(" \n-------------------------------------------  \n");

    // Angles from MPU
     Serial.print("AngleRoll:");
     Serial.print(AngleRoll);
     Serial.print("  ");
     Serial.print("AnglePitch:");
     Serial.print(AnglePitch);
    Serial.println(" \n-------------------------------------------  \n");

    delay(500);

    while (micros() - LoopTimer < (t * 1000000))

    {
        LoopTimer = micros();
    }
}
