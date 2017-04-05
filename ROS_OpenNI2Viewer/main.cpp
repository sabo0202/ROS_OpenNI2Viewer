#include <iostream>
#include <list>

#include <OpenNI.h>
#include <opencv.hpp>
#include <glut.h>

// OpenNIストリームの初期化
void initStream( openni::VideoStream& stream )
{
    openni::VideoMode videoMode;
    if (stream.isValid())
    {
        videoMode = stream.getVideoMode();
        videoMode.setFps( 30 );
        videoMode.setResolution( 640, 480 );
    }
    
    openni::Status ret = stream.setVideoMode( videoMode );
    if ( ret != openni::STATUS_OK ) {
        throw std::runtime_error( "stream.setVideoMode" );
    }
    
    stream.setMirroringEnabled(true);
}


// カラーストリームを表示できる形に変換する
cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
{
    // OpenCV の形に変換する
    cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                 colorFrame.getWidth(),
                                 CV_8UC3, (unsigned char*)colorFrame.getData() );
    
    // BGR の並びを RGB に変換する
    cv::cvtColor( colorImage, colorImage, CV_RGB2BGR );
    
    return colorImage;
}

// Depthイメージを可視化
cv::Mat convertDepthToColor( openni::VideoFrameRef& depthFrame )
{
    openni::VideoMode video_mode = depthFrame.getVideoMode();
    cv::Mat depthImage = cv::Mat(video_mode.getResolutionY(),
                                 video_mode.getResolutionX(),
                                 CV_16U, (char*)depthFrame.getData());
    depthImage.convertTo(depthImage, CV_8UC1, 255.0/10000);
    //depthImage = 255 - depthImage; // 近い方を白に
    
    return depthImage;
}


int main(int argc, char** argv)
{
    openni::Status ret = openni::STATUS_OK;
    
    openni::Device device;
    openni::VideoStream colorStream;
    openni::VideoStream depthStream;
    std::vector<openni::VideoStream*> streams;
    const char* deviceURI = openni::ANY_DEVICE;
    if (argc > 1)
    {
        deviceURI = argv[1];
    }
    
    //auto version = openni::OpenNI::getVersion();
    //std::cout << "OpenNI " << version.major << "." << version.minor << "." <<
    //version.maintenance << "." << version.build << std::endl;
        
    ret = openni::OpenNI::initialize();
    printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());
    if ( ret != openni::DEVICE_STATE_OK ) {
        throw std::runtime_error( "openni::OpenNI::initialize" );
    }
    
    ret = device.open( deviceURI );
    if ( ret != openni::DEVICE_STATE_OK ) {
        printf("SimpleViewer: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
        openni::OpenNI::shutdown();
        return 1;
    }
        
    // Grab Detector のドキュメントより
    // Depth と Color の解像度は VGA(640x480)とし、Depth と Color のフレームを同期させること
    //ColorStreamの作成
    ret = colorStream.create(device, openni::SENSOR_COLOR);
    if (ret == openni::STATUS_OK)
    {
        //initStream( colorStream );
        ret = colorStream.start();
        if (ret != openni::STATUS_OK)
        {
            printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
            colorStream.destroy();
        }
    }
    else
    {
        printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
    }
    
    //DepthStreamの作成
    ret = depthStream.create(device, openni::SENSOR_DEPTH);
    if (ret == openni::STATUS_OK)
    {
        //initStream( depthStream );
        ret = depthStream.start();
        if (ret != openni::STATUS_OK)
        {
            printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
            depthStream.destroy();
        }
    }
    else
    {
        printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
    }
    
    if (!depthStream.isValid() || !colorStream.isValid())
    {
        printf("SimpleViewer: No valid streams. Exiting\n");
        openni::OpenNI::shutdown();
        return 2;
    }
    
    if (ret != openni::STATUS_OK)
    {
        openni::OpenNI::shutdown();
        return 3;
    }
    
    // DepthとColorを同期
    device.setDepthColorSyncEnabled( true );
    
    // DepthとColorの位置合わせ
    device.setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );
    
    // ÉXÉgÉäÅ[ÉÄÇÇ‹Ç∆ÇﬂÇÈ
    streams.push_back( &colorStream );
    streams.push_back( &depthStream );

    // ÉÅÉCÉìÉãÅ[Év
    
    cv::Mat depthImage;
    cv::Mat colorImage;
    
    cv::Mat colorImageShow(1200, 800, CV_8UC3);
    cv::Mat depthImageShow(1200, 800, CV_8UC1);
        
    while ( 1 ) {

        do {
            // depth Ç®ÇÊÇ— color ÉtÉåÅ[ÉÄÇéÊìæÇ∑ÇÈ
            openni::VideoFrameRef depthFrame;
            depthStream.readFrame( &depthFrame );
            depthImage = convertDepthToColor( depthFrame );
            
            openni::VideoFrameRef colorFrame;
            colorStream.readFrame( &colorFrame );
            colorImage = showColorStream( colorFrame );
            
        } while (colorImage.empty() && depthImage.empty());
        
        //画像を拡大
        //cv::resize(colorImage, colorImageShow, cv::Size(1200, 800));
        //cv::resize(depthImage, depthImageShow, cv::Size(1200, 800));
        
        //cv::imshow( "Depth Image", depthImage );
        cv::imshow( "Color Image", colorImage );
        
        //拡大画像を表示
        //cv::imshow( "Depth Image", depthImageShow );
        //cv::imshow( "Color Image", colorImageShow );
        
        int key = cv::waitKey( 10 );
        if ( key == 'q' ) {
            //depthStream.stop();
            //colorStream.stop();
            //depthStream.destroy();
            //colorStream.destroy();
            //device.close();
            //openni::OpenNI::shutdown();
            break;
        }
    }
    
    return 0;
}


/*
#include <iostream>
#include <list>

#include <OpenNI.h>
#include <opencv.hpp>

cv::Mat depthImage;
cv::Mat colorImage;

class kinectUI
{
public:
    
    // init OpenNI
    void initOpenNI()
    {
        openni::Status ret = openni::STATUS_OK;
        
        auto version = openni::OpenNI::getVersion();
        std::cout << "OpenNI " << version.major << "." << version.minor << "." <<
        version.maintenance << "." << version.build << std::endl;
        
        ret = openni::OpenNI::initialize();
        if ( ret != openni::DEVICE_STATE_OK ) {
            throw std::runtime_error( "openni::OpenNI::initialize" );
        }
        
        ret = device.open( openni::ANY_DEVICE );
        if ( ret != openni::DEVICE_STATE_OK ) {
            throw std::runtime_error( "device.open" );
        }
        
        // Grab Detector のドキュメントより
        // Depth と Color の解像度は VGA(640x480)とし、Depth と Color のフレームを同期させること
        
        // Color ÉXÉgÉäÅ[ÉÄÇçÏê¨Ç∑ÇÈ
        ret = colorStream.create( device, openni::SensorType::SENSOR_COLOR );
        if ( ret != openni::STATUS_OK ) {
            throw std::runtime_error( "colorStream.create" );
        }
        
        //initStream( colorStream );
        ret = colorStream.start();
        if ( ret != openni::STATUS_OK ) {
            throw std::runtime_error( "colorStream.start" );
        }
        
        // Depth ÉXÉgÉäÅ[ÉÄÇçÏê¨Ç∑ÇÈ
        ret = depthStream.create( device, openni::SensorType::SENSOR_DEPTH );
        if ( ret != openni::STATUS_OK ) {
            throw std::runtime_error( "depthStream.create" );
        }
        
        //initStream( depthStream );
        ret = depthStream.start();
        if ( ret != openni::STATUS_OK ) {
            throw std::runtime_error( "depthStream.start" );
        }
        
        // Depth Ç∆ Color ÇÃÉtÉåÅ[ÉÄÇìØä˙Ç≥ÇπÇÈ
        device.setDepthColorSyncEnabled( true );
        
        // Depth Ç Color ÇÃç¿ïWÇ…çáÇÌÇπÇÈ
        device.setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );
        
        // ÉXÉgÉäÅ[ÉÄÇÇ‹Ç∆ÇﬂÇÈ
        streams.push_back( &colorStream );
        streams.push_back( &depthStream );
    }
    
    // OpenNI ÉXÉgÉäÅ[ÉÄÇÃèâä˙âª
    void initStream( openni::VideoStream& stream )
    {
        auto videoMode = stream.getVideoMode();
        videoMode.setFps( 30 );
        videoMode.setResolution( 640, 480 );
        auto ret = stream.setVideoMode( videoMode );
        if ( ret != openni::STATUS_OK ) {
            throw std::runtime_error( "stream.setVideoMode" );
        }
        
        stream.setMirroringEnabled(true);
    }
    
    
    // ÉÅÉCÉìÉãÅ[Év
    void run()
    {
        
        
        while ( 1 ) {
            
            do {
                // depth Ç®ÇÊÇ— color ÉtÉåÅ[ÉÄÇéÊìæÇ∑ÇÈ
                openni::VideoFrameRef depthFrame;
                depthStream.readFrame( &depthFrame );
                depthImage = convertDepthToColor( depthFrame );
                
                openni::VideoFrameRef colorFrame;
                colorStream.readFrame( &colorFrame );
                colorImage = showColorStream( colorFrame );
                
            } while (colorImage.empty() && depthImage.empty());
            
            //glutInit(&argc, argv);
            glutInitDisplayMode(GLUT_RGBA);
            glutInitWindowSize(640, 640);
            glutCreateWindow("TEST");
            
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0.0, 640, 640, 0.0);
            glViewport(0, 0, 640, 640);
            
            glutDisplayFunc(display);
            
            glutMainLoop();
            
            //cv::imshow( "Depth Image", depthImage );
            //cv::imshow( "Color Image", colorImage );
            
            int key = cv::waitKey( 10 );
            if ( key == 'q' ) {
                break;
            }
        }
    }
    
    // カラーストリームを表示できる形に変換する
    cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
    {
        // OpenCV の形に変換する
        cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                     colorFrame.getWidth(),
                                     CV_8UC3, (unsigned char*)colorFrame.getData() );
        
        // BGR の並びを RGB に変換する
        cv::cvtColor( colorImage, colorImage, CV_RGB2BGR );
        
        return colorImage;
    }
    
    // Depth ÉfÅ[É^ÇÉJÉâÅ[âÊëúÇ…ïœä∑Ç∑ÇÈ
    cv::Mat convertDepthToColor( openni::VideoFrameRef& depthFrame )
    {
        openni::VideoMode video_mode = depthFrame.getVideoMode();
        cv::Mat depthImage = cv::Mat(video_mode.getResolutionY(),
                                    video_mode.getResolutionX(),
                                    CV_16U, (char*)depthFrame.getData());
        depthImage.convertTo(depthImage, CV_8UC1, 255.0/10000);
        depthImage = 255 - depthImage; // 近い方を白に
        
        return depthImage;
    }
    
    static void display(void)
    {
        //cv::Mat img = cv::imread("test.jpg");
        cv::flip(colorImage, colorImage, 0);
        cv::cvtColor(colorImage, colorImage, CV_BGR2RGB);
        
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(colorImage.cols, colorImage.rows,
                     GL_RGB, GL_UNSIGNED_BYTE, colorImage.data);
        glFlush();
    }
    
private:
    
    openni::Device device;
    
    openni::VideoStream colorStream;
    openni::VideoStream depthStream;
    std::vector<openni::VideoStream*> streams;
    
};

int main()
{
    try {
        kinectUI app;
        app.initOpenNI();
        app.run();
    }
    catch ( std::exception& ) {
        std::cout << openni::OpenNI::getExtendedError() << std::endl;
    }
}
*/
