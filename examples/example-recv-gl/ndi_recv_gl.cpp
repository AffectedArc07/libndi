#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <list>
#include <ndi.h>

#ifdef __arm__
//#include "bcm_host.h"
#endif

#include "../common/ogl.h"
#include "../common/yuv.h"

#include <mutex>
#include <string>

std::list<ndi_packet_video_t*> _queue;
std::mutex _mutex;

void recv_thread(ndi_recv_context_t recv_ctx) {

    ndi_packet_video_t video;
    ndi_packet_audio_t audio;
    ndi_packet_metadata_t meta;

    while (ndi_recv_is_connected(recv_ctx)) {
        
        int data_type = ndi_recv_capture(recv_ctx, &video, &audio, &meta, 1000);
        switch (data_type) {
                
            case NDI_DATA_TYPE_VIDEO:
                //printf("Video data received (%dx%d %.4s).\n", video.width, video.height, (char*)&video.fourcc);
            {
                ndi_packet_video_t * clone = (ndi_packet_video_t*)malloc(sizeof(ndi_packet_video_t));
                memcpy(clone, &video, sizeof(ndi_packet_video_t));
                _mutex.lock();
                while (_queue.size() > 2) {
                    ndi_packet_video_t * v = _queue.back();
                    _queue.pop_back();
                    ndi_recv_free_video(v);
                    free(v);
                }
                _queue.push_back(clone);
                _mutex.unlock();
            }
                break;
                
            case NDI_DATA_TYPE_AUDIO:
                //printf("Audio data received (%d samples).\n", audio.num_samples);
                ndi_recv_free_audio(&audio);
                break;
                
            case NDI_DATA_TYPE_METADATA:
                printf("Meta data received: %s\n", meta.data);
                ndi_recv_free_metadata(&meta);
                break;
        }
    }
}


int main(int argc, char* argv[]) {

	int ret;

#ifdef __arm__
	//bcm_host_init();
#endif

    // Initialize OpenGL
	if ((ret = init_ogl(0)) < 0) {
		printf("Failed to initialize OpenGL. Error: %d\n", ret);
		return -1;
	}

	bool filter_sources = false;
	std::string source_filter;

	if (argc > 1) {
		filter_sources = true;
		source_filter = argv[1];
		printf("Source filter: %s\n", argv[1]);
	}

	// NDI find source
	ndi_find_context_t find_ctx = ndi_find_create();
	ndi_source_t  * sources = NULL;
	int nb_sources = 0;
    bool valid_source = false;
	ndi_source_t target_source;
	printf("Started source search\n");
	while (!valid_source) {
		sources = ndi_find_sources(find_ctx, 5000, &nb_sources);

		if(nb_sources > 0) {
			if(filter_sources) {
				// Only filtered source
				for (int i = 0; i < sizeof(sources); i++) {
					std::string target_filter = sources[i].name;
					if(target_filter == source_filter) {
						printf("Source found.\n");
						target_source = sources[i];
						valid_source = true;
						break;
					}
				}
			} else {
				// Just take the first
				target_source = sources[0];
				valid_source = true;
			}
		}		
	}

	printf("Found source: %s (%s:%d)\n", target_source.name, target_source.ip, target_source.port);

	int width, height;
	res_ogl(0, &width, &height);
	printf("Screen #0: %dx%d\n", width, height);

    // Create Window
	printf("Attempting to create window\n");
	if ((ret = window_ogl(0, width, height)) < 0) {
		printf("Failed to create window. Error: %d\n", ret);
		return -1;
	}

	printf("Enabling textures\n");

	glEnable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 1);

#ifndef __arm__
	glewInit();
#endif

	printf("Enabling shaders\n");

    // Load YUV shader
	if (yuv_init() < 0) {
		printf("Failed to load YUV shader\n");
		return -1;
	}

	printf("Binding shaders\n");
	yuv_bind();

    // NDI receive
	ndi_recv_context_t recv_ctx = ndi_recv_create();
	ret = ndi_recv_connect(recv_ctx, target_source.ip, target_source.port);
	if (ret < 0) {
		printf("Failed to connect to source\n");
		return -1;
	}
    printf("Connected.\n");

	ndi_find_free(find_ctx);
    
    std::thread t(recv_thread, recv_ctx);
    
    // NDI codec
	ndi_codec_context_t codec_ctx = ndi_codec_create();
    ndi_frame_t frame;
    ndi_video_format_t format;

    // Main loop
	while (loop_ogl()) {

		int width, height;
		size_ogl(0, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
        //glLoadIdentity();
        
        // Pop most recent frame off the queue
        ndi_packet_video_t * video = NULL;
        if (_queue.size() > 0) {
            _mutex.lock();
            video = _queue.front();
            _queue.pop_front();
            _mutex.unlock();
        }

        // Decode video if available
        if (video != NULL) {
            frame = ndi_codec_decode(codec_ctx, video);
            if (frame) {
                ndi_frame_get_format(frame, &format);
                for (int i = 0; i < format.num_planes; i++) {
                    void * data = ndi_frame_get_data(frame, i);
                    int w = i ? format.chroma_width : format.width;
                    int h = i ? format.chroma_height : format.height;
                    int linesize = ndi_frame_get_linesize(frame, i);
                    yuv_data(i, (unsigned char*)data, w, h, linesize);
                }
                ndi_frame_free(frame);
            }
            ndi_recv_free_video(video);
            free(video);
        }

        yuv_view(0, width, 0, height, -1, 1);
        yuv_draw(0, 0, width, height);

		redraw_ogl(0);
	}

    ndi_recv_close(recv_ctx);
    t.join();
    ndi_recv_free(recv_ctx);

	exit_ogl();

	return 0;
}
