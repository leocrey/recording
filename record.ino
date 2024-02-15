#include <Audio.h>
#include <SDHCI.h>

AudioClass *theAudio;
SDClass theSD;
File myFile;

static void audio_attention_cb(const ErrorAttentionParam *atprm) {
  Serial.println("Attention!");
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
    Serial.println("Error or warning occurred.");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  if (!theSD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  Serial.println("Audio system initialized.");

  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 1000, 160, false);
  theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", 44100, AS_CHANNEL_STEREO);
}

void loop() {
  // Use a static variable to ensure the file name is set once per recording session
  static bool isRecording = false;
  static unsigned long startTime = 0;

  if (!isRecording) {
    // Generate a unique file name for each recording session
    String fileName = "REC_" + String((unsigned long)millis()) + ".wav";

    myFile = theSD.open(fileName.c_str(), FILE_WRITE);
    if (!myFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
    
    theAudio->writeWavHeader(myFile);
    Serial.println("WAV header written. File: " + fileName);

    theAudio->startRecorder();
    Serial.println("Recording started.");
    startTime = millis();
    isRecording = true;
  } else if (millis() - startTime > 30000) { // 30 seconds
    theAudio->stopRecorder();
    theAudio->readFrames(myFile);

    myFile.close();
    Serial.println("Recording stopped and file closed.");

    isRecording = false;
    // No need to reset startTime or reopen the file for the next session,
    // as the new session will generate a new file name.
  }
}



