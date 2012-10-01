/*
 *  ofxTLAudioTrack.h
 *  audiowaveformTimelineExample
 *
 *  Created by Jim on 12/28/11.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxTLTrack.h"
#include "ofOpenALSoundPlayer_TimelineAdditions.h"

class ofxTLAudioTrack : public ofxTLTrack
{
  public:	
	ofxTLAudioTrack();
	virtual ~ofxTLAudioTrack();
	
	virtual void draw();
	
	virtual bool loadSoundfile(string filepath);
	virtual bool isSoundLoaded();
	virtual float getDuration(); //in seconds
	virtual string getSoundfilePath();
	
	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);
	
	virtual void keyPressed(ofKeyEventArgs& args);
	
	//this will play the timeline along to the audio
    virtual bool togglePlay();
    virtual void play();
    virtual void stop();
    virtual bool getIsPlaying();
	
	vector<float>& getFFTSpectrum(int numBins);
	int getDefaultBinCount();

	virtual void zoomStarted(ofxTLZoomEventArgs& args);
	virtual void zoomDragged(ofxTLZoomEventArgs& args);
	virtual void zoomEnded(ofxTLZoomEventArgs& args);
	
	virtual void boundsChanged(ofEventArgs& args);
	
	virtual void setSpeed(float speed);
    virtual float getSpeed();
	virtual void setVolume(float volume);
	virtual void setPan(float pan);
    
	virtual string getTrackType();
	
  protected:	
    bool soundLoaded;
	bool shouldRecomputePreview;
	vector<ofPolyline> previews;
	void recomputePreview();
	string soundFilePath;
	float lastFFTPosition;
	int defaultFFTBins;
	vector<float> fftBins;
	float lastPercent;
	virtual void update(ofEventArgs& args);
	ofOpenALSoundPlayer_TimelineAdditions player;
	ofRange computedZoomBounds;
	float maxBinReceived;
};
