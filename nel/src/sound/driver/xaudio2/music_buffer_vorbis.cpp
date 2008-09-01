/**
 * \file music_buffer_vorbis.cpp
 * \brief CMusicBufferVorbis
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * CMusicBufferVorbis
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2008  Jan Boon (Kaetemi)
 * 
 * This file is part of NLSOUND Music Library.
 * NLSOUND Music Library is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * NLSOUND Music Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NLSOUND Music Library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include "stdxaudio2.h"
#include "music_buffer_vorbis.h"

// Project includes

// NeL includes
#include <nel/misc/stream.h>
#include <nel/misc/file.h>

// STL includes

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

size_t vorbisReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	CMusicBufferVorbis *music_buffer_vorbis = (CMusicBufferVorbis *)datasource;
	NLMISC::IStream *stream = music_buffer_vorbis->getStream();
	nlassert(stream->isReading());
	uint32 length = size * nmemb;
	if (length > music_buffer_vorbis->getStreamSize() - stream->getPos())
		length = music_buffer_vorbis->getStreamSize() - stream->getPos();
	stream->serialBuffer((uint8 *)ptr, length);
	return length;
}

int vorbisSeekFunc(void *datasource, ogg_int64_t offset, int whence)
{
	if (whence == SEEK_CUR && offset == 0)
	{
		// nlwarning("This seek call doesn't do a damn thing, wtf.");
		return 0; // ooookkaaaaaayyy
	}

	CMusicBufferVorbis *music_buffer_vorbis = (CMusicBufferVorbis *)datasource;

	NLMISC::IStream::TSeekOrigin origin;
	switch (whence)
	{
	case SEEK_SET:
		origin = NLMISC::IStream::begin;
		break;
	case SEEK_CUR:
		origin = NLMISC::IStream::current;
		break;
	case SEEK_END:
		origin = NLMISC::IStream::end;
		break;
	default:
		// nlwarning("Seeking to fake origin.");
		return -1;
	}

	if (music_buffer_vorbis->getStream()->seek((sint32)offset, origin)) return 0;
	else return -1;
}

//int vorbisCloseFunc(void *datasource)
//{
//	//CMusicBufferVorbis *music_buffer_vorbis = (CMusicBufferVorbis *)datasource;
//}

long vorbisTellFunc(void *datasource)
{
	CMusicBufferVorbis *music_buffer_vorbis = (CMusicBufferVorbis *)datasource;
	return (long)music_buffer_vorbis->getStream()->getPos();
}

static ov_callbacks OV_CALLBACKS_NLMISC_STREAM = {
  (size_t (*)(void *, size_t, size_t, void *))  vorbisReadFunc,
  (int (*)(void *, ogg_int64_t, int))          vorbisSeekFunc,
  (int (*)(void *))                             NULL, //vorbisCloseFunc,
  (long (*)(void *))                            vorbisTellFunc
};

CMusicBufferVorbis::CMusicBufferVorbis(NLMISC::IStream *stream, bool loop) 
: _Stream(stream), _Loop(loop), _StreamSize(0), _IsMusicEnded(false)
{
	sint pos = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(pos, NLMISC::IStream::begin);
	ov_open_callbacks(this, &_OggVorbisFile, NULL, 0, OV_CALLBACKS_NLMISC_STREAM);
}

CMusicBufferVorbis::~CMusicBufferVorbis()
{
	ov_clear(&_OggVorbisFile);
}

bool CMusicBufferVorbis::getSongTitle(const std::string &fileName, NLMISC::IStream *stream, std::string &result)
{
	CMusicBufferVorbis mbv(stream, false); // just opens and closes the oggvorbisfile thing :)
	vorbis_comment *vc = ov_comment(&mbv._OggVorbisFile, -1);
	char *title, *artist;
	title = vorbis_comment_query(vc, "title", 0);
	artist = vorbis_comment_query(vc, "artist", 0);
	if (title)
	{
		if (artist)
		{
			result = std::string(artist) + " - " + std::string(title);
			
		}
		else
		{
			result = std::string(title);
		}
	}
	else if (artist)
	{
		result = std::string(artist) + " - " + fileName;
	}
	else
	{
		result = fileName;
	}
	return true;
}

uint32 CMusicBufferVorbis::getRequiredBytes()
{
	return 0; // no minimum requirement of bytes to buffer out
}

uint32 CMusicBufferVorbis::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	int current_section = 0; // ???
	nlassert(!_IsMusicEnded); // shouldn't be called anymore normally when ended
	nlassert(minimum <= maximum); // can't have this..
	uint32 bytes_read = 0;
	do
	{
		// signed 16-bit or unsigned 8-bit little-endian samples
		int br = ov_read(&_OggVorbisFile, (char *)&buffer[bytes_read], maximum - bytes_read, 
			0, // Specifies big or little endian byte packing. 0 for little endian, 1 for b ig endian. Typical value is 0.
			getBitsPerSample() == 8 ? 1 : 2, 
			getBitsPerSample() == 8 ? 0 : 1, // Signed or unsigned data. 0 for unsigned, 1 for signed. Typically 1.
			&current_section);
		// nlinfo("current_section: %i", current_section);
		if (br <= 0) 
		{ 
			if (br == 0)
			{
				if (_Loop)
				{
					ov_pcm_seek(&_OggVorbisFile, 0);
					//_Stream->seek(0, NLMISC::IStream::begin);
				}
				else 
				{
					_IsMusicEnded = true;
					break; 
				}
			}
			else
			{
				nlwarning("ov_read: %i", br);
			}
		}
		else bytes_read += (uint32)br;
	} while (bytes_read < minimum);
	return bytes_read;
}

uint16 CMusicBufferVorbis::getChannels()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	return vi->channels;
}

uint32 CMusicBufferVorbis::getSamplesPerSec()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	return vi->rate;
}

uint16 CMusicBufferVorbis::getBitsPerSample()
{
	return 16;
}

bool CMusicBufferVorbis::isMusicEnded()
{
	return _IsMusicEnded;
}

float CMusicBufferVorbis::getLength()
{
	return (float)ov_time_total(&_OggVorbisFile, -1);
}

} /* namespace NLSOUND */

/* end of file */
