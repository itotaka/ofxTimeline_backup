/*
 *  ofxTLAudioTrack.cpp
 *  audiowaveformTimelineExample
 *
 *  Created by Jim on 12/28/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxTLAudioTrack.h"
#include "ofxTimeline.h"

ofxTLAudioTrack::ofxTLAudioTrack(){
	shouldRecomputePreview = false;
    soundLoaded = false;
	lastFFTPosition = 0;
	defaultFFTBins = 256;
	maxBinReceived = 0;
}

ofxTLAudioTrack::~ofxTLAudioTrack(){

}

bool ofxTLAudioTrack::loadSoundfile(string filepath){
	soundLoaded = false;
	if(player.loadSound(filepath, false)){
    	soundLoaded = true;
		soundFilePath = filepath;
		shouldRecomputePreview = true;
    }
	return soundLoaded;
}
 
string ofxTLAudioTrack::getSoundfilePath(){
	return soundFilePath;
}

bool ofxTLAudioTrack::isSoundLoaded(){
	return soundLoaded;
}

float ofxTLAudioTrack::getDuration(){
	return player.getDuration();
}

void ofxTLAudioTrack::update(ofEventArgs& args){
	if(player.getPosition() < lastPercent){
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackLooped, args);
	}
	lastPercent = player.getPosition();
	
    //currently only supports timelines with duration == duration of player
	if(lastPercent < timeline->getInOutRange().min){
		player.setPosition(timeline->getInOutRange().min);
	}
	else if(lastPercent > timeline->getInOutRange().max){
		if(timeline->getLoopType() == OF_LOOP_NONE){
			player.setPosition(timeline->getInOutRange().max);
			stop();
		}
		else{
			player.setPosition(timeline->getInOutRange().min);
		}
	}
	
    timeline->setCurrentTimeSeconds(player.getPosition() * player.getDuration());
}
 
void ofxTLAudioTrack::draw(){
	
	if(!soundLoaded || player.getBuffer().size() == 0){
		ofPushStyle();
		ofSetColor(timeline->getColors().disabledColor);
		ofRectangle(bounds);
		ofPopStyle();
		return;
	}
		
	if(shouldRecomputePreview || viewIsDirty){
		recomputePreview();
	}


    ofPushStyle();
    ofSetColor(timeline->getColors().keyColor);
    ofNoFill();
    
    for(int i = 0; i < previews.size(); i++){
        ofPushMatrix();
        ofTranslate( normalizedXtoScreenX(computedZoomBounds.min, zoomBounds)  - normalizedXtoScreenX(zoomBounds.min, zoomBounds), 0, 0);
        ofScale(computedZoomBounds.span()/zoomBounds.span(), 1, 1);
        previews[i].draw();
        ofPopMatrix();
    }
    ofPopStyle();
	
	if(getIsPlaying() || timeline->getIsPlaying()){
		ofPushStyle();
		
		//will refresh fft bins for other calls too
		vector<float>& bins = getFFTSpectrum(defaultFFTBins);
		float binWidth = bounds.width / bins.size();
		//find max
		float averagebin = 0 ;
		for(int i = 0; i < bins.size(); i++){
			maxBinReceived = MAX(maxBinReceived, bins[i]);
			averagebin += bins[i];
		}
		averagebin /= bins.size();
		
		ofFill();
		ofSetColor(timeline->getColors().disabledColor, 120);
		for(int i = 0; i < bins.size(); i++){
			float height = bounds.height * bins[i]/maxBinReceived;
			float y = bounds.y + bounds.height - height;
			ofRect(i*binWidth, y, binWidth, height);
		}
		
		ofPopStyle();
	}
	
}

void ofxTLAudioTrack::recomputePreview(){
	
	previews.clear();
	
//	cout << "recomputing view with zoom bounds of " << zoomBounds << endl;
	
	float normalizationRatio = timeline->getDurationInSeconds() / player.getDuration(); //need to figure this out for framebased...but for now we are doing time based
	float trackHeight = bounds.height/(1+player.getNumChannels());
	int numSamples = player.getBuffer().size() / player.getNumChannels();
	int pixelsPerSample = numSamples / bounds.width;
	for(int c = 0; c < player.getNumChannels(); c++){
		ofPolyline preview;
		int lastFrameIndex = 0;
		for(float i = bounds.x; i < bounds.x+bounds.width; i++){
			float pointInTrack = screenXtoNormalizedX( i, zoomBounds ) * normalizationRatio; //will scale the screenX into wave's 0-1.0
			float trackCenter = bounds.y + trackHeight * (c+1);
			if(pointInTrack <= 1.0){
				//draw sample at pointInTrack * waveDuration;
				int frameIndex = pointInTrack * numSamples;					
				float losample = 0;
				float hisample = 0;
				for(int f = lastFrameIndex; f < frameIndex; f++){
					int sampleIndex = f * player.getNumChannels() + c;
					float subpixelSample = player.getBuffer()[sampleIndex]/32565.0;
					if(subpixelSample < losample) {
						losample = subpixelSample;
					}
					if(subpixelSample > hisample) {
						hisample = subpixelSample;
					}
				}
				if(losample == 0 && hisample == 0){
					preview.addVertex(i, trackCenter);
				}
				else {
					if(losample != 0){
						preview.addVertex(i, trackCenter - losample * trackHeight);
					}
					if(hisample != 0){
						//ofVertex(i, trackCenter - hisample * trackHeight);
						preview.addVertex(i, trackCenter - hisample * trackHeight);
					}
				}
				lastFrameIndex = frameIndex;
			}
			else{
				preview.addVertex(i,trackCenter);
			}
		}
		preview.simplify();
		previews.push_back(preview);
	}
	computedZoomBounds = zoomBounds;
	shouldRecomputePreview = false;
}

int ofxTLAudioTrack::getDefaultBinCount(){
//	cout << defaultFFTBins << endl;
	return defaultFFTBins;
}

bool ofxTLAudioTrack::mousePressed(ofMouseEventArgs& args, long millis){
	return false;
}

void ofxTLAudioTrack::mouseMoved(ofMouseEventArgs& args, long millis){
}

void ofxTLAudioTrack::mouseDragged(ofMouseEventArgs& args, long millis){
}

void ofxTLAudioTrack::mouseReleased(ofMouseEventArgs& args, long millis){
}

void ofxTLAudioTrack::keyPressed(ofKeyEventArgs& args){
}

void ofxTLAudioTrack::zoomStarted(ofxTLZoomEventArgs& args){
	ofxTLTrack::zoomStarted(args);
//	shouldRecomputePreview = true;
}

void ofxTLAudioTrack::zoomDragged(ofxTLZoomEventArgs& args){
	ofxTLTrack::zoomDragged(args);
	//shouldRecomputePreview = true;
}

void ofxTLAudioTrack::zoomEnded(ofxTLZoomEventArgs& args){
	ofxTLTrack::zoomEnded(args);
	shouldRecomputePreview = true;
}

void ofxTLAudioTrack::boundsChanged(ofEventArgs& args){
	shouldRecomputePreview = true;
}

void ofxTLAudioTrack::play(){
	if(!player.getIsPlaying()){
		
//		lastPercent = MIN(timeline->getPercentComplete() * timeline->getDurationInSeconds() / player.getDuration(), 1.0);
		player.setLoop(timeline->getLoopType() == OF_LOOP_NORMAL);
//		cout << "calling play on track " << endl;
		player.play();
		player.setPosition(timeline->getPercentComplete());
		ofAddListener(ofEvents().update, this, &ofxTLAudioTrack::update);
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackStarted, args);		
	}	   
}

void ofxTLAudioTrack::stop(){
	if(player.getIsPlaying()){

//		cout << "calling stop on track " << endl;
		player.setPaused(true);
		ofRemoveListener(ofEvents().update, this, &ofxTLAudioTrack::update);
		
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackEnded, args);
	}
}

bool ofxTLAudioTrack::togglePlay(){
	if(getIsPlaying()){
		stop();
	}
	else {
		play();
	}
	return getIsPlaying();
}

bool ofxTLAudioTrack::getIsPlaying(){
    return player.getIsPlaying();
}

void ofxTLAudioTrack::setSpeed(float speed){
    player.setSpeed(speed);
}

float ofxTLAudioTrack::getSpeed(){
    return player.getSpeed();
}

void ofxTLAudioTrack::setVolume(float volume){
    player.setVolume(volume);
}

void ofxTLAudioTrack::setPan(float pan){
    player.setPan(pan);
}

vector<float>& ofxTLAudioTrack::getFFTSpectrum(int numBins){
	float fftPosition = player.getPosition();
	if(isSoundLoaded() && lastFFTPosition != fftPosition){
		if(defaultFFTBins != numBins){
			maxBinReceived = 0;
			defaultFFTBins = numBins;
		}
		lastFFTPosition = fftPosition;
		fftBins = player.getSpectrum(defaultFFTBins);
	}
	return fftBins;
}

string ofxTLAudioTrack::getTrackType(){
    return "Audio";    
}

   
