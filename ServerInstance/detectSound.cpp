/*
 * detectSound.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: fcaldas
 */

#include "ServerInstance.h"
#include "../recognition/imageRecognition.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <queue>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

namespace RestServer {

	/*
	 * Get RAW sound data, this will return an interleaved signed short
	 * audio file with 2 channels.
	 *
	 * You can transform it into an wav file using sox:
	 * sox -r48000 -e signed-integer -b 16 -c 2 DATA.raw out.wav
	 */
	void wwwgetSound(web::http::http_request request){
		void *data;
		char *p;
		int sz;
		ServerInstance::cameraDeckLink->getAudioData(&data, &sz);
		p = (char *)data;
		std::string binsound(p , p + sz);
		free(p);
		request.reply(status_codes::OK, binsound, "Content-type: text/binary");
	}

	void wwwcheckAudio(web::http::http_request request){
		void *data;
		json::value reply;
		int sz;
		bool hasAudio = false ;
		ServerInstance::cameraDeckLink->getAudioData(&data, &sz);
		sz = sz/sizeof(short);
		hasAudio = imageRecognition::bufferHasAudio((short*) data, sz);
		reply["hasAudio"] = json::value::boolean(hasAudio);
		free(data);
		request.reply(status_codes::OK, reply);
	}

	void wwwaudioStatus(http_request request) {
		json::value resp = request.extract_json().get();
		json::value answer;
		int timeAnalysis;
		if (resp.has_field("timeAnalysis") && resp["timeAnalysis"].is_number()) {
			if (resp["timeAnalysis"].as_integer() > 400) {
				timeAnalysis = resp["timeAnalysis"].as_integer();

				bool audioStatus = getAudioState(timeAnalysis);
				answer["isAudioPresent"] = web::json::value::boolean(audioStatus);
			} else {
				answer["error"] = 1;
				answer["message"] = web::json::value::string(
						"Variable 'timeAnalysis' has to be greater than 400ms");
			}

		} else {
			answer["error"] = 1;
			answer["message"] =
					web::json::value::string(
							"This request needs one value: 'timeAnalysis' the time in milliseconds that the video output will be evaluated");
		}
		request.reply(status_codes::OK, answer);
	}


} /* Namespace RestServer */
