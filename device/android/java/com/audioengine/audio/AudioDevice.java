package com.snail.audio;

import java.nio.ByteBuffer;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.media.MediaRecorder.AudioSource;
import android.os.Process;
import android.util.Log;
import android.os.Build;

public class AudioDevice { 
	
	private class device_info
	{
		public int               source = 0;
		public int               minbufsize = 0;
		public int               channel = 0;
		public int               samprate = 0;   
		public int               frame_time = 0;
	}
	//error_code
	public static final int ERR_OK = 0; 
	public static final int ERR_NOT_FOUND_RECORD_DEVICE = ERR_OK+1;
	public static final int ERR_NOT_FOUND_PLAYOUT_DEVICE = ERR_NOT_FOUND_RECORD_DEVICE+1;
	public static final int ERR_ALLOC_MEMORY_FAILED = ERR_NOT_FOUND_PLAYOUT_DEVICE+1; 
	public static final int ERR_OPEN_RECORD_DEVICE_FAILED = ERR_ALLOC_MEMORY_FAILED+1;
	public static final int ERR_OPEN_PLAYOUT_DEVICE_FAILED = ERR_OPEN_RECORD_DEVICE_FAILED+1; 
	public static final int ERR_INIT_RECORD_DEVICE_FAILED = ERR_OPEN_PLAYOUT_DEVICE_FAILED+1;
	public static final int ERR_INIT_PLAYOUT_DEVICE_FAILED = ERR_INIT_RECORD_DEVICE_FAILED+1;
	public static final int ERR_START_RECORD_FAILED = ERR_INIT_PLAYOUT_DEVICE_FAILED+1;
	public static final int ERR_STOP_RECORD_FAILED = ERR_START_RECORD_FAILED+1; 
	public static final int ERR_START_PLAYOUT_FAILED = ERR_STOP_RECORD_FAILED+1;
	public static final int ERR_STOP_PLAYOUT_FAILED = ERR_START_PLAYOUT_FAILED+1;
	public static final int ERR_READ_RECORD_DATA_FAILED = ERR_STOP_PLAYOUT_FAILED+1;
	public static final int ERR_WRITE_PLAYOUT_DATA_FAILED = ERR_READ_RECORD_DATA_FAILED+1;
	
	final private int  bitrate = AudioFormat.ENCODING_PCM_16BIT; 
	
	
	private obj_audio_info    m_info = new obj_audio_info();
    private device_info       m_rec_params = new device_info();
    private device_info       m_ply_params = new device_info();
    private boolean           m_hard_aec_flag = false;
	 
	private AudioEffectEx      m_effect = null; 
	private AudioRecordThread  m_record_thread = null;
	private AudioPlayoutThread m_play_thread = null;
	private boolean            m_bfound_record = false;
	private boolean            m_runflag  = false;
	private int                m_support_source = -1;
	private long               m_ins = 0;
	private ByteBuffer m_RecordbyteBuffer;
	private ByteBuffer m_PlayoutbyteBuffer;
	private String m_strBuildInfo;
 	public native void    ON_RecordData(long ins,int len);    
 	public native void    ON_NeedMorePlayData(long ins, int len); 
 	public native void    CacheDirectBufferAddress( long nativeAudioRecord,ByteBuffer RecordbyteBuffer,ByteBuffer PlayoutbyteBuffer);
	public native void    ErrorReport(long ins,int ec);
 	public AudioDevice()
	{
		m_info.playout_channel  = 0;
		m_info.playout_samprate = 0;
		m_info.record_channel   = 0;
		m_info.recrord_samprate = 0; 
		m_runflag = false; 
	}

		
	
	public boolean IO_Init(long ins)
	{
		if(m_runflag)
			return false;
		boolean bInit = true;
		m_ins = ins;
		if(!check_recformat())
		{
			ErrorReport(ins,ERR_NOT_FOUND_RECORD_DEVICE);
			bInit = false;
		}
		if(!check_playformat())
		{
			ErrorReport(ins,ERR_NOT_FOUND_PLAYOUT_DEVICE);
			bInit = false;
		}
		
		m_info.record_channel    = (AudioFormat.CHANNEL_IN_STEREO == m_rec_params.channel) ? 2: 1;
		m_info.recrord_samprate  = m_rec_params.samprate;
	
		m_info.playout_channel    = (AudioFormat.CHANNEL_IN_STEREO == m_ply_params.channel) ? 2: 1;
		m_info.playout_samprate  = m_ply_params.samprate;

		return bInit;
	}
	public int IO_Get(int type)
	{
		switch(type)
		{
		case 1:
			return m_info.recrord_samprate;
		case 2:
			return m_info.playout_samprate;
		case 3:
			return m_info.record_channel;
		case 4:
			return m_info.playout_channel;
		case 5:
			return m_rec_params.source;
		case 6:
			return m_ply_params.source;
		case 7:
			return m_support_source;
		default:
			return 0;
		}
	}
	
	public boolean IO_Start(int audio_source, int rec_frame_time, int play_frame_time)
	{
		if(m_runflag)
			return false;
		  
		m_runflag = true;
	    final int recbytesPerFrame = m_info.record_channel * (16 / 8);
	    final int recframesPerBuffer = m_info.recrord_samprate / 1000 * rec_frame_time;
	    final int plybytesPerFrame = m_info.playout_channel * (16 / 8);
	    final int plyframesPerBuffer = m_info.playout_samprate / 1000 * play_frame_time;
	    m_RecordbyteBuffer = ByteBuffer.allocateDirect(recbytesPerFrame * recframesPerBuffer);
	    m_PlayoutbyteBuffer = ByteBuffer.allocateDirect(plybytesPerFrame * plyframesPerBuffer);
	    CacheDirectBufferAddress(m_ins,m_RecordbyteBuffer,m_PlayoutbyteBuffer);
	    
		StartRecording(audio_source, rec_frame_time); 
		StartPlayout(play_frame_time); 
		return true;
	}
	
	public void IO_Stop()
	{
		if(!m_runflag)
			return; 
		
		m_runflag = false;
		StopRecording();
		StopPlayout(); 
	}
	
	public String IO_GetDeviceInfo()
	{
		m_strBuildInfo =String.format("c:%s|id:%s|v:%s",Build.BRAND,Build.DEVICE,Build.VERSION.RELEASE );
		return m_strBuildInfo;
	}
	public boolean StartRecording(int audio_source, int frame_time) { 
		
		    Log.d("AudioDevice", "record StopRecording---------------------");
		 
		    m_bfound_record = false;
		    m_rec_params.frame_time = frame_time; 
	    	Log.d("AudioDevice", "audio record StartRecording, samprate:" + m_rec_params.samprate + " frame:" + frame_time);
	    	
	    	if(0 == m_rec_params.samprate){
	    		Log.d("AudioDevice", "audio record StartRecording failed");
	    		return false;
	    	}
	    	
	    	if( audio_source != -1 )
	    	{
	    		m_support_source = audio_source;
	    	}
	    	else{
	    		m_support_source = m_rec_params.source;
	    	}
	    	
	    	m_record_thread = new AudioRecordThread(m_support_source, "AudioDevice_record_thread");
	    	m_record_thread.start();
	    	m_bfound_record = true;
	    		
	    	check_aec(m_record_thread.GetRecoder());

		    return true;
		  }
		
		 private void StopRecording() { 
			Log.d("AudioDevice", "audio record StopRecording ..."); 
			 
			if(null != m_record_thread)
				m_record_thread.Stop();  
			
		    if (m_effect != null) {
		    	m_effect.release();
		    	m_effect = null;
		    } 
			m_record_thread = null;  
			
			Log.d("AudioDevice", "audio record StopRecording ok"); 
	   }
		 
		 public boolean StartPlayout(int frame_time) {  
 
			Log.d("AudioDevice", "audio playout StartPlayout, samprate:" + m_ply_params.samprate + " frame:" + frame_time);
			m_ply_params.frame_time = frame_time;   
	    	if(0 == m_ply_params.samprate){
	    		Log.d("AudioDevice", "audio playout StartPlayout failed");
	    		return false;
	    	}
	    	 
	    	m_play_thread      = new AudioPlayoutThread(m_ply_params.source, "AudioDevice_playout_thread"); 
	    	m_play_thread.start(); 
		    return true; 
		  }
		
		 public void StopPlayout() { 
			 
			Log.d("AudioDevice", "audio playout StopPlayout..."); 
			
			if(null != m_play_thread){
				m_play_thread.Stop();  			 
				m_play_thread = null;  
			}
			
			Log.d("AudioDevice", "audio playout StopPlayout ok"); 
	   }
	 
	 public boolean check_recformat(){ // 
			 
			boolean bfound  = false;
			
			for(int rec_resource : new int[]{MediaRecorder.AudioSource.VOICE_COMMUNICATION, MediaRecorder.AudioSource.CAMCORDER, MediaRecorder.AudioSource.MIC, MediaRecorder.AudioSource.DEFAULT}){
				for (int sampleRate : new int[]{48000, 32000, 16000, 44100,22050, 11025, 8000}){      
				// 
					for (short channelConfig : new short[]{AudioFormat.CHANNEL_IN_MONO,AudioFormat.CHANNEL_IN_STEREO}) {
					  try {
	                      int nBufSize = AudioRecord.getMinBufferSize(sampleRate, channelConfig, bitrate); 
	                      if (nBufSize < 0) 
	                           continue; 
	                       
	                      AudioRecord dev = new AudioRecord(rec_resource, sampleRate, channelConfig, bitrate, nBufSize); 
	                       
	                      if (dev.getState() == AudioRecord.STATE_INITIALIZED){  
	                           m_rec_params.channel     = channelConfig;
	                           m_rec_params.samprate    = sampleRate;
	                           m_rec_params.source      = rec_resource;
	                           m_rec_params.minbufsize  = nBufSize; 
	                           bfound = true;
	                      }
	 
	                      dev.release();                      
	                      dev = null;  
	                      
	                      if(bfound){
	                    	Log.d("AudioDevice", "audio record support sampreate: " + m_rec_params.samprate + " channel: " + m_rec_params.channel + " rec_resource: " + m_rec_params.source);
	          				return true;
	                      }
					  	} catch (Exception e) {   
					  		Log.d("AudioDevice:", "audio record " + e.getMessage());
					  	}              
					}//end for 
				}//end for 
			}//end for
			 
			Log.d("AudioDevice", "audio record check, not found samplerate "); 
			return false;
			
		}//end function
	 
     public boolean IsBuildinAECSupported()
     {
    	 return m_hard_aec_flag;
     }
     
	 public boolean check_playformat(){ //  
			boolean bfound = false;
				 
			for(int ply_resource : new int[]{AudioManager.STREAM_VOICE_CALL, AudioManager.STREAM_MUSIC}){
				for (int sampleRate : new int[]{48000, 32000, 16000, 44100, 22050, 11025, 8000}){      
					// 
					for (short channelConfig : new short[]{AudioFormat.CHANNEL_OUT_STEREO,AudioFormat.CHANNEL_OUT_MONO}) {
						  try {
		                        int nBufSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, bitrate); 
		                        if (nBufSize < 0)
		                             continue;
		                        
		                        AudioTrack dev = new AudioTrack(ply_resource, sampleRate, channelConfig, bitrate, nBufSize, AudioTrack.MODE_STREAM); 
		                        
		                        if (dev.getState() == AudioTrack.STATE_INITIALIZED){  
			                           m_ply_params.channel     = channelConfig;
			                           m_ply_params.samprate    = sampleRate;
			                           m_ply_params.source      = ply_resource;
			                           m_ply_params.minbufsize  = nBufSize;
			                           bfound = true;
		                        }
		                        dev.release();                      
		                        dev = null;   
		                        
		                      if(bfound){
		                    	Log.d("AudioDevice", "audio playout support sampreate: " + m_ply_params.samprate + " channel: " + m_ply_params.channel + " ply_resource: " + m_ply_params.source);
		          				return true;
			                  }
		                        
		                  } catch (Exception e) {    
		                	  Log.e("AudioDevice:", "audio playout " + e.getMessage());
		                  }              
				     }//end for 
				  }//end for  
			}//end for
			
		    Log.d("AudioDevice", "audio playout check, not found samplerate "); 
			return false;
			
		}//end function
	 
	 
	 private void check_aec(AudioRecord record){
		 try{
             m_effect = new AudioEffectEx(record); 
			 if(m_effect.setEnabled(true)){
				 m_hard_aec_flag = true;
			 }
			 else{ 
				 m_hard_aec_flag = false;
				 m_effect.release();
				 m_effect = null;
			 }
		 }catch(Exception e)
		 {
			 Log.d("AudioDevice", "check aec fail " + e.getMessage());
		 }
	 }
 
	 
	    //audio record
	    private class AudioRecordThread extends Thread { 
		    private AudioRecord      dev                 = null;
            private int              framesize           = 0;
		    private boolean          runflag             = false;
		    private int              source              = 0;
		    private boolean          running              = false;
		    public AudioRecord GetRecoder(){
		    	return dev;
		    }

		    public AudioRecordThread(int rec_source, String name) {
		      super(name);  
		      source         = rec_source;
		      framesize      = m_rec_params.frame_time*2*m_info.record_channel*m_rec_params.samprate/1000; 
		     
		      int buffersize = Math.max(m_rec_params.minbufsize,  framesize);  
		      
		      Log.d("AudioDevice", "audio record, buffersize:  " + buffersize  + 
		               " framesize:" + framesize +
		               " samprate: " + m_rec_params.samprate +
		               " source: " + rec_source); 
		      

		      try{
		          dev = new AudioRecord(source, m_rec_params.samprate, m_rec_params.channel, bitrate, buffersize);
		      }
		      catch(Exception e){
		    	  ErrorReport(m_ins,ERR_OPEN_RECORD_DEVICE_FAILED);
		    	  try{
		    		  dev = new AudioRecord(m_rec_params.source, m_rec_params.samprate, m_rec_params.channel, bitrate, buffersize);	   
		    	  }catch(Exception exp){
				      Log.e("AudioDevice", "audio record device open failed, buffersize:  " + buffersize  + 
				               " framesize:" + framesize +
				               " samprate: " + m_rec_params.samprate +
				               " source: " + rec_source);
				      ErrorReport(m_ins,ERR_OPEN_RECORD_DEVICE_FAILED);
				      return;
		    	  }
		      }

		      runflag        = true;
		    }
		    
		    public void work(){ 
		    	// start
		    	if (dev.getState() != AudioRecord.STATE_INITIALIZED){
		    		Log.d("AudioDevice", "audio record init failed, source: " + dev.getAudioSource());
		    		ErrorReport(m_ins,ERR_INIT_RECORD_DEVICE_FAILED);
		    		return;
		    	}
		    	
		    	try{
		    	   dev.startRecording(); 
		    	}
		    	catch(IllegalStateException e){
		    		ErrorReport(m_ins,ERR_START_RECORD_FAILED);
		    		Log.d("AudioDevice", "audio record startRecording failed, source: " + dev.getAudioSource());
		    	    return;
		    	}
		    	 
		    	// read data

			      int     nreadfrom = 0;
			      int     nreadlen  = framesize; 
			      while (runflag) { 
			    	  int bytesRead = dev.read(m_RecordbyteBuffer, m_RecordbyteBuffer.capacity());
			          if (bytesRead > 0) { 
			        	  nreadfrom += bytesRead;
			        	  nreadlen  -= bytesRead;
			        	  if(0 == nreadlen){
			        		  ON_RecordData(m_ins, framesize );
			        		  nreadfrom = 0;
			        		  nreadlen  = framesize;
			        	  } 
			          } 
			          
			          if(bytesRead ==  AudioRecord.ERROR_INVALID_OPERATION )
			          {
			        	  Log.e("AudioDevice","AudioRecord.read failed: " + bytesRead);
			        	  ErrorReport(m_ins,ERR_READ_RECORD_DATA_FAILED);
			        	  runflag = false;
			          }
			      }
			      
			      
			      //stop
			      try { 
				        dev.stop();
				      } catch (IllegalStateException e) {
				    	  ErrorReport(m_ins,ERR_STOP_RECORD_FAILED);
				    	  Log.e("AudioDevice", "audio record.stop failed: " + e.getMessage());
				  } 
			       
		    }

		    @Override
		    public void run() { 
		      if(null == dev)
		    	  return;
		      
		      running = true;
		      Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
		      
              work();
		      
		      running = false;
		    }
		    
		    public void Stop(){
		       if(!runflag)
		    	   return;
		       
		       runflag = false;
		       
		       while (isAlive()) {
		           try {
		             join();
		           } catch (InterruptedException e) {
		             // Ignore.
		        	   ErrorReport(m_ins,ERR_STOP_RECORD_FAILED);
		           }
		         }
		    }
 
		  } 
	    
	    
	    
	    //audio playout
	    private class AudioPlayoutThread extends Thread { 
		    private AudioTrack      dev                 = null;
            private int              framesize           = 0;
		    private boolean          runflag             = false;
		    private boolean          running             = false;  
		    private int              source              = 0; 
		     

		    public AudioPlayoutThread(int ply_source, String name) {
		      super(name);  
		      source         = ply_source; 
		      runflag = true; 
		    }
		    
		    public void work(){
		    	
			      framesize      = m_ply_params.frame_time*2*m_info.playout_channel*m_ply_params.samprate/1000;  
			      int buffersize = Math.max(m_ply_params.minbufsize,  framesize);  
			      
			      Log.d("AudioDevice", "audio playout, buffersize:  " + buffersize  + 
			               " framesize:" + framesize +
			               " samprate: " + m_rec_params.samprate +
			               " source: " + source); 
			      
		    	// start 
		    	try{
		    	   dev = new AudioTrack(source, m_ply_params.samprate, m_ply_params.channel, bitrate, buffersize, AudioTrack.MODE_STREAM);  
		    	   if (dev.getState() != AudioTrack.STATE_INITIALIZED){
			    		Log.d("AudioDevice", "audio playout init failed, source: " + source);
			    		ErrorReport(m_ins,ERR_INIT_PLAYOUT_DEVICE_FAILED);
			    		return;
			    	} 
		    	    dev.play(); 
		    	}
		    	catch(IllegalStateException e){ 
		    		Log.d("AudioDevice", "audio playout failed, source: " + source);
		    		ErrorReport(m_ins,ERR_START_PLAYOUT_FAILED);
		    	    return;
		    	}
		    	


			   
			      while (runflag) { 

			        ON_NeedMorePlayData(m_ins, framesize);
			    	int bytesWritten = dev.write(m_PlayoutbyteBuffer.array(), m_PlayoutbyteBuffer.arrayOffset(), m_PlayoutbyteBuffer.capacity());
			    	if(bytesWritten != framesize){
                        Log.e("AudioDevice","AudioTrack.write failed: " + bytesWritten);
    			        if (bytesWritten == AudioTrack.ERROR_INVALID_OPERATION) {
    			        	ErrorReport(m_ins,ERR_WRITE_PLAYOUT_DATA_FAILED);
  			              runflag = false;
  			            }
			    	}

			        m_PlayoutbyteBuffer.rewind();
			      }
			      
			      //stop
			      try {
				        dev.stop();
				        dev.flush();
				      } catch (IllegalStateException e) {
				    	  Log.e("AudioDevice", "audio playout.stop failed: " + e.getMessage());
				    	  ErrorReport(m_ins,ERR_STOP_PLAYOUT_FAILED);
				  } 
		    }

		    @Override
		    public void run() {  
		      running = true;
		      Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO); 
		      work();  
		      running = false;
		    }
		    
		    public void Stop(){
		       if(!runflag)
		    	   return;
		       
		       runflag = false;
		       
		       while (isAlive()) {
		           try {
		             join();
		           } catch (InterruptedException e) {
		             // Ignore.
		           }
		         }
		    }
 
		  } 
}
