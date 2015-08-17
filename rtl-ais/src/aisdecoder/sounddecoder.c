/*
 *    sounddecoder.cpp
 *
 *    This file is part of AISDecoder.
 *
 *    Copyright (C) 2013
 *      Astra Paging Ltd / AISHub (info@aishub.net)
 *
 *    AISDecoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    AISDecoder uses parts of GNUAIS project (http://gnuais.sourceforge.net/)
 *
 */

#include <string.h>
#include <stdio.h>
//#include "config.h"
#ifdef WIN32
	#include <fcntl.h>
#endif

#include "receiver.h"
#include "hmalloc.h"
#include "shared_vars.h"
#define MAX_FILENAME_SIZE 512
#define ERROR_MESSAGE_LENGTH 1024
#include "sounddecoder.h"
#include "convenience.h"

char errorSoundDecoder[ERROR_MESSAGE_LENGTH];


static short *buffer=NULL;
static int buffer_l=0;
static int buffer_read=0;
static int channels=0;
static Sound_Channels sound_channels;
static FILE *fp=NULL;
static void readBuffers();
static time_t tprev=0;
static time_t tprev_ppm=0;
static int time_print_stats=0;
static int thread_running_a=0;
static int prev_a_receivedframes = 0;
static int prev_a_lostframes = 0;
static int prev_a_lostframes2 = 0;
static int prev_b_receivedframes = 0;
static int prev_b_lostframes = 0;
static int prev_b_lostframes2 = 0;
static int ppm_auto_time=30;

int initSoundDecoder(int buf_len,int _time_print_stats) 
{
	sound_channels=SOUND_CHANNELS_STEREO;
	channels = sound_channels == SOUND_CHANNELS_MONO ? 1 : 2;
	time_print_stats=_time_print_stats;
	tprev=time(NULL); // for decoder statistics
    tprev_ppm=time(NULL); 
    buffer = (short *) hmalloc(channels*sizeof(short)*buf_len);
    rx_a = init_receiver('A', 2, 0);
    rx_b = init_receiver('B', 2, 1);
    return 1;
}

void run_mem_decoder(short * buf, int len,int max_buf_len)
{	
	int offset=0;
	int bytes_in_len=len*channels;
	char * p=(char *) buf;
//      rtlsdr_dev_t *dev ;
//      int ppm_error ;

	while(bytes_in_len > max_buf_len )
	{
		memcpy(buffer,p+offset,max_buf_len);
		buffer_read=max_buf_len/(channels*sizeof(short));
		bytes_in_len-=max_buf_len;
		offset+=max_buf_len;
		readBuffers();
	}
	memcpy(buffer,p+offset,bytes_in_len);
	buffer_read=bytes_in_len/(channels*sizeof(short));
	readBuffers();
	
	if(time_print_stats && (time(NULL)-tprev >= time_print_stats))
	{
		struct demod_state_t *d = rx_a->decoder;
		tprev=time(NULL);
		fprintf(stderr,
				"A: Received correctly: %d packets, wrong CRC: %d packets, wrong size: %d packets\n",
				d->receivedframes, d->lostframes,
				d->lostframes2);
		d = rx_b->decoder;
			fprintf(stderr,
				"B: Received correctly: %d packets, wrong CRC: %d packets, wrong size: %d packets\n",
				d->receivedframes, d->lostframes,
				d->lostframes2);
	}
// 	Do auto tuning. Checking both channels at regular intervals and calc the
// 	percentage of correct received messages. At correct tuning this should be 
// 	for both channels more ore less equal
	

	if(ppm_auto_time && (time(NULL)-tprev_ppm >= ppm_auto_time))
    {
        struct demod_state_t *da = rx_a->decoder;
        struct demod_state_t *db = rx_b->decoder;

                
        int rx_a_frames = da->receivedframes - prev_a_receivedframes;
        int rx_a_lostframes = da->lostframes - prev_a_lostframes;
        int rx_a_lostframes2 = da->lostframes2 - prev_a_lostframes2;
        int rx_b_frames = db->receivedframes - prev_b_receivedframes;
        int rx_b_lostframes = db->lostframes - prev_b_lostframes;
        int rx_b_lostframes2 = db->lostframes2 - prev_b_lostframes2;
        // check if enough messages are received to do a decent check
        if ( ( rx_a_frames + rx_b_frames ) >= 10 ) 
        {
            //save values for next calculation
            prev_a_receivedframes = da->receivedframes;
            prev_a_lostframes =  da->lostframes;
            prev_a_lostframes2 = da->lostframes2;
            prev_b_receivedframes = db->receivedframes;
            prev_b_lostframes =  db->lostframes;
            prev_b_lostframes2 = db->lostframes2;
            int perc_a;
            int perc_b;
            if ( (rx_a_frames + rx_a_lostframes + rx_a_lostframes2 ) == 0 )
                perc_a = 0;
            else
                perc_a = (int)(rx_a_frames * 100 / ( rx_a_frames + rx_a_lostframes + rx_a_lostframes2 )) ;
            if ( rx_b_frames + rx_b_lostframes + rx_b_lostframes2 == 0 )
                perc_b = 0;
            else
                perc_b = (int)(rx_b_frames * 100 / ( rx_b_frames + rx_b_lostframes + rx_b_lostframes2 ))  ;
            
            int fr_p_min = (int)( rx_a_frames + rx_b_frames )*60 / (time(NULL)-tprev_ppm);
             
            int ppm_corr = (int) round((perc_b - perc_a)/10);
            if (ppm_corr > 3) ppm_corr = 3;
            if (ppm_corr < -3) ppm_corr = -3;
            if ( ppm_corr != 0 )
            {   
                ppm_error = ppm_error + ppm_corr;
                verbose_ppm_set(dev, ppm_error);
            }
            else
                ppm_auto_time = ppm_auto_time * 2;
            if ( ppm_auto_time > 300 )
                ppm_auto_time = 300;
                
            //if(time_print_stats)
                fprintf(stderr,"Sentences per minuut %i, quality Ch.A %i%% Ch.B %i%% ppm error set to %i \n", fr_p_min, perc_a, perc_b, ppm_error );
            
            tprev_ppm=time(NULL);            
        }
        //int curr_ppm=
    }
}
void runSoundDecoder(int *stop) {
    while (!*stop) {
        buffer_read = fread(buffer, channels * sizeof(short), buffer_l, fp);
        readBuffers();
    }
}

static void readBuffers() {
    if (buffer_read <= 0) return;
    if (rx_a != NULL && sound_channels != SOUND_CHANNELS_RIGHT)
    {       
        receiver_run(rx_a, buffer, buffer_read);
        
    }

    if (rx_b != NULL &&
        (sound_channels == SOUND_CHANNELS_STEREO || sound_channels == SOUND_CHANNELS_RIGHT)   ) 
        receiver_run(rx_b, buffer, buffer_read);
}

void freeSoundDecoder(void) {
    if (fp != NULL) {
        fclose(fp);
        fp=NULL;
    }

    if (rx_a != NULL) {
        free_receiver(rx_a);
        rx_a=NULL;
    }

    if (rx_b != NULL) {
        free_receiver(rx_b);
        rx_b=NULL;
    }
    if (buffer != NULL) {
        hfree(buffer);
        buffer = NULL;
    }
}
