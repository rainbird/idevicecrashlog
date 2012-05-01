idevicecrashlog
---------------
This simple command line tool let you retrive a crash log from your iOS device.<br/> 

Is is based on [libimobiledevice](http://www.libimobiledevice.org/). 

With [jembe](http://www.jembe.fr), you can develop your iOS application from linux, windows or mac. 

On the other side, there are some project based on libimobiledevice that make it easy to develop for iOS from linux. 
These tools are ideviceinstaller to install an ipa, and idevicesyslog that let you see the syslogs in realtime. 

But, when developping an iOS application, you may need to access the crash report.

installation
------------

To compile use: 

    clang -o idevicecrashlog idevicecrashlog.c -limobiledevice -larchive
    
how to use it
-------------

When debugguing your app with idevicesyslog, if it crash, you will receive the message: 


    May  1 21:53:29 Jembe-iPad SpringBoard[15] <Warning>: Application 'creation' exited abnormally with signal 11: Segmentation fault: 11
    May  1 21:53:29 Jembe-iPad ReportCrash[6406] <Error>: Saved crashreport to /var/mobile/Library/Logs/CrashReporter/creation_2012-05-01-215328_My-iPad.plist using uid: 0 gid: 0, synthetic_euid: 501 egid: 0
    
And to get the crash report:

    idevicecrashlog creation_2012-05-01-215328_My-iPad.plist

