/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video_recorder.h>

using namespace pangolin;
using namespace std;

void RecordSample(const std::string input_uri, const std::string record_uri)
{
    // Setup Video Source
    VideoInput video(input_uri);
    const VideoPixelFormat vid_fmt = video.PixFormat();
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    VideoOutput recorder( record_uri );
    recorder.AddStream(w,h, "YUV420P");

    // Create Glut window
    pangolin::CreateWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    // Allocate image buffer. The +1 is to give ffmpeg some alignment slack
    // swscale seems to have a bug which goes over the array by 1...
    unsigned char* img = new unsigned char[video.SizeBytes() + 1];
    
    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if( video.GrabNext(img,true) )
        {
            // Upload to GPU as texture for display
            texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);

            // Record video frame
            recorder[0].WriteImage(img, w, h, vid_fmt);
        }

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front and process window events via GLUT
        pangolin::FinishFrame();
    }

    delete[] img;
}

int main( int argc, char* argv[] )
{
    std::string record_uri = "ffmpeg:[fps=30,bps=8388608]//video.avi";

    std::string input_uris[] = {
        "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0",
        "convert:[fmt=RGB24]//v4l:///dev/video0",
        "convert:[fmt=RGB24]//v4l:///dev/video1",
        ""
    };

    if( argc >= 2 ) {
        const string uri = std::string(argv[1]);
        if( argc == 3 ) {
            record_uri = std::string(argv[2]);
        }
        RecordSample(uri, record_uri);
    }else{
        cout << "Usage  : SimpleRecord [video-uri] [output-uri]" << endl << endl;
        cout << "Where video-uri describes a stream or file resource, e.g." << endl;
        cout << "\tfile:[realtime=1]///home/user/video/movie.pvn" << endl;
        cout << "\tfile:///home/user/video/movie.avi" << endl;
        cout << "\tfiles:///home/user/seqiemce/foo%03d.jpeg" << endl;
        cout << "\tdc1394:[fmt=RGB24,size=640x480,fps=30,iso=400,dma=10]//0" << endl;
        cout << "\tdc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0" << endl;
        cout << "\tv4l:///dev/video0" << endl;
        cout << "\tconvert:[fmt=RGB24]//v4l:///dev/video0" << endl;
        cout << "\tmjpeg://http://127.0.0.1/?action=stream" << endl;
        cout << endl;

        // Try to open some video device
        for(int i=0; !input_uris[i].empty(); ++i )
        {
            try{
                cout << "Trying: " << input_uris[i] << endl;
                RecordSample(input_uris[i], record_uri);
                return 0;
            }catch(VideoException) {}
        }
    }

    return 0;

}
