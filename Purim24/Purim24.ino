// Arduino sketch to test the UtraSonic sensor
#include <NewPing.h>
#include <Servo.h>

const int redPin = 8;
const int orangePin = 9;
const int greenPin = 10;

const int trigPin = 12;
const int echoPin = 11;
const int servoPin = 3;
const int speakerPin = 6;

const int maxDistance = 200;
const int maxDistances[] = {maxDistance/4, maxDistance/2, 3*maxDistance/4};

const int MODE = 0;

Servo servo;
int servo_angle = 0;

NewPing sonar(trigPin, echoPin, maxDistance);

void setLEDs(int state) {
    digitalWrite(redPin, state);
    digitalWrite(orangePin, state);
    digitalWrite(greenPin, state);
}

void blinkLEDs(int delayTime) {
    // digitalWrite(LED_BUILTIN, HIGH);
    setLEDs(HIGH);
    delay(delayTime);
    // digitalWrite(LED_BUILTIN, LOW);
    setLEDs(HIGH);
    delay(delayTime);
}

int getDistance() {
    int distance = sonar.ping_cm(0);
    while (distance == 0) {
        distance = sonar.ping_cm(0);
        delay(50);
    }
    return distance;
}

class DistanceGame {
public:
    int score;
    int minTargetTimeMillis;
    int target;
    int error;
    int distance;
    int start_time;

    DistanceGame() {
        score = 0;
        minTargetTimeMillis = 4000;
        target = random(10, 200);
        start_time = 0;
    }

    void success() {
        score++;
        target = random(10, 200);
        start_time = 0;
        for (int i = 0; i < score; i++) {
            setLEDs(HIGH);
            delay(200);
            setLEDs(LOW);
            delay(200);
        }
    }

    void play() {
        while (true) {
            distance = getDistance();
            error = abs(distance - target);
            setLEDs(LOW);
            if (error < 1) {
                digitalWrite(greenPin, HIGH);
                if (start_time == 0) {
                    start_time = millis();
                    continue;
                }
                if (millis() - start_time > minTargetTimeMillis) {
                    success();
                    return;
                }
            } else {
                start_time = 0;
                if (error < 5) {
                    digitalWrite(orangePin, HIGH);
                } else {
                    digitalWrite(redPin, HIGH);
                    digitalWrite(orangePin, LOW);
                }
            }
        }
        delay(200);
    }
};

class DigitalPiano {
public:
    const int max_distance_cm = 100;
    const int min_distance_cm = 5;
    int speakerPin;
    bool mode;  // 0: piano, 1: game
    const int notesLength = 12;
    int notes[36] = { 261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493,
                      523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987,
                      1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975 };

    // only C D ... without the black keys

    // const int notesLength = 24;
    // int notes[35] = {
    //     131, 147, 165, 175, 196, 220, 247,
    //     261, 293, 329, 349, 392, 440, 493,
    //     523, 587, 659, 698, 783, 880, 987,
    //     1046, 1174, 1318, 1396, 1567, 1760, 1975,
    // };

    DigitalPiano(int speakerPin, bool mode) {
        this->speakerPin = speakerPin;
        this->mode = mode;

        pinMode(speakerPin, OUTPUT);
    }

    void loop() {
        if (mode == 0) {
            readNote();
            // delay(50);
        } else {
            play();
        }
    }

    void playSequence(int sequence[], int length) {
        for (int i = 0; i < length; i++) {
            tone(speakerPin, notes[sequence[i]], 100);
            delay(100);
            noTone(speakerPin);
            // delay(100);
        }
    }

    int readNote() {
        int distance = getDistance();
        int note = -1;
        if (distance <= max_distance_cm && distance >= min_distance_cm) {
            note = map(distance, min_distance_cm, max_distance_cm, notesLength, 0);
            tone(speakerPin, notes[note], 100);
            delay(30);
        } else {
            // shut up
            noTone(speakerPin);
        }
        return note;
    }
    
    void play() {
        // play random notes, add one at each round
        const int game_rounds = 10;
        int sequence[game_rounds] = {};
        for (int i = 0; i < game_rounds; i++) {
            sequence[i] = random(0, notesLength);
            playSequence(sequence, 1);
            delay(500);
            // wait for user input
            // read the distance, and allow for a small error and give time to react
            // if wrong, play the sequence again
            // if correct, continue
            //
            for (int j = 0; j < i; j++) {
                if (sequence[j] != readNote()) {
                    playSequence(sequence, i);
                    mode = 0;
                    return;
                }
            }
        }
    }
};

void trafficLight(int distance) {
  // print("Distance: %d\n", distance);
    if (distance < maxDistances[0]) {
        digitalWrite(redPin, HIGH);
        digitalWrite(orangePin, LOW);
        digitalWrite(greenPin, LOW);
    } else {
        if (distance < maxDistances[1]) {
            digitalWrite(orangePin, HIGH);
            digitalWrite(redPin, LOW);
            digitalWrite(greenPin, LOW);
        } else {
            digitalWrite(orangePin, LOW);
            if (distance < maxDistances[2]) {
                digitalWrite(greenPin, HIGH);
                digitalWrite(redPin, LOW);
                digitalWrite(orangePin, LOW);
            } else {
                digitalWrite(greenPin, LOW);
            }
        }
    }
}



// --------------------------------------------------------------
DigitalPiano piano(speakerPin, 0);
void setup() {
    Serial.begin(9600);

    // pinMode(LED_BUILTIN, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(orangePin, OUTPUT);
    pinMode(greenPin, OUTPUT);

    pinMode(servoPin, OUTPUT);
    servo.attach(servoPin);
    servo.write(servo_angle);

    pinMode(speakerPin, OUTPUT);
}

void loop1() {
    piano.loop();
}

void loop() {
    if (MODE == 0) {
        piano.loop();
        delay(50);
        return;
    }

    int distance = getDistance();
    trafficLight(distance);

    int new_angle = map(distance, 0, maxDistance/2, 45, 135);

    // set the servo angle
    if (abs(new_angle - servo_angle) > 1) {
        servo_angle = new_angle;
    }
    if (servo_angle > 180) {
        servo_angle = 180;
    } else if (servo_angle < 0) {
        servo_angle = 0;
    }
    servo.write(servo_angle);
    delay(50);
}


// DistanceGame game;
// void loop() {
//     game.play();
// }

