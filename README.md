# WBOT02 - Qt WOTBOT 

In [TBOT01](https://github.com/caiks/TBOT01#readme), [TBOT02](https://github.com/caiks/TBOT02#readme) and [TBOT03](https://github.com/caiks/TBOT03#readme) we investigated real-time *modelling* of a Turtlebot, a small wheeled robot with a lidar. We experimented with it by running various controllers in the [turtlebot3 house](http://emanual.robotis.com/docs/en/platform/turtlebot3/simulation/#3-turtlebot3-house) simulated in the [Gazebo](http://gazebosim.org/) virtual environment. Now we shall take a step in a different direction - Wotbot will focus on real-time *modelling* of vision, hearing and speech implemented in a smartphone. That is, Wotbot will be a robot with a head, rather than a body, at least in its initial versions.

`TBOT01` was the semi-static preparatory step - it (1) acquired experience, (2) modelled, and (3) acted in three different stages. The *model* was trained using the *inducers* and *conditioners* implemented in the [AlignmentRepaC repository](https://github.com/caiks/AlignmentRepaC). The AlignmentRepaC repository is a fast C++ implementation of some of the *practicable inducers* described in the paper *The Theory and Practice of Induction by Alignment* at https://greenlake.co.uk/. The AlignmentRepaC repository was tested on the MNIST handwritten digits dataset in  [NISTC](https://github.com/caiks/NISTC#readme). `TBOT01` was able to use its statically learned *model* to find its way around the turtlebot house faster than chance.

In `TBOT02` we implemented fully dynamic *modelling* - instead of acquiring *history*, *modelling*, and *applying model* separately, these tasks are done concurrently. The active functionality was implemented in the [AlignmentActive repository](https://github.com/caiks/AlignmentActive), which depends on the [AlignmentRepaC repository](https://github.com/caiks/AlignmentRepaC). `TBOT02` demonstrated that active *modelling* was no worse than static *induced modelling*, although not as good as static *conditional modelling* (where there is an available label). `TBOT02` also showed that active *modelling* can process *events* in real-time with moderate compute and limited memory requirements, while maintaining high *model likelihoods*. The active framework can concurrently manage multiple active *models* structured in different *levels*. Higher *levels* can include time-wise *underlying* and reflexive *frames*. Active *histories* also allow the temporal analysis of past *events* and *slice* transistions; thus *slice* topologies can provide a mechanism for exploration and control.

In `TBOT03` we sought to improve *modelling* performance by searching for *model likelihood* in the *slice* topology as an indicator of potential new *model*. In 'interest' mode the turtlebot acted to move towards the *slice* with the greatest *size* per parent *slice size* of its surrounding *slices*. That is, it tended to move towards *on-diagonal slices*. In 'unusual' mode the turtlebot acted to move towards the *slice* with the least *size* per parent *slice size*. That is, it moved instead towards far *off-diagonal slices*. 
We were able to demonstrate that deliberately searching for interesting or unusual *slices*, rather than acting to attain a labelled goal, can accelerate *model* growth, at least in the case of limited active *history size*. 

The turtlebot house and other simulated environments are hugely simplified models of the real world, however accurate the physics. In fact, the turtlebot house was so simplified that, even with added furniture to help the turtlebot distinguish between rooms, the lidar sensor did not carry enough *alignments* to remove the location ambiguities. Ironically, the *slice* topology of `TBOT03` was now so complete and connected that it did not accidentally miss any of the wormholes or inconsistent mappings to its physical space. In fact, the navigation performance of `TBOT03` is far worse than chance!

Given these location ambiguities, it seems unlikely that a complex limbed robot with camera vision would have an environment in a simulated virtual world that is rich enough to enable a later transition to real world embodiment. On the other hand, if we attempt to develop active *models* using real robots instead of virtual ones, we are presented with very difficult technical challenges. So, to make progress, we shall move directly to the real world but initially without locomotion or manipulation. To do this we shall use the existing technologies of cameras, microphones and speakers in smartphones and laptops. It will be up to the user to carry wotbot around and point it in the general direction of something interesting.

In this repository, `WBOT02` has a *substrate* derived from images sourced from a camera, a video or a screen-grab. A typical RGB image from one of these sources has thousands to millions of pixels with the colour of each pixel encoded in 24 bits. In order to allow for practicable action *induction*, the wotbot *substrate* has only 40x40 *variables* each with a *valency* of 10 *values*. To reduce the image we choose a centre (a pixel coordinate) and a scale (e.g. 1.0, 0.5, 0.25 ...) so that only a portion of the image is captured - something like the saccades of the human eye. Where a *variable* corresponds to a rectangle of pixels, an average pixel value is calculated. The Red-Green-Blue colour is converted to a 256 grey-scale by taking the maximum of the red, green or blue components (the 'value' of Hue-Saturation-Value). The 256 *valency* is then reduced to 10 by calculating the deciles or by simply dividing into regular intervals. 

We can compare this process of reduction to the case of the MNIST handwritten digits dataset described in [Haskell analysis](https://greenlake.co.uk/pages/dataset_NIST) or [Python analysis](https://greenlake.co.uk/pages/dataset_python_NIST). There the images were already centered and scaled and reduced to 28x28 pixel *variables* of 256 *values*. We then only had to bucket the *variables*; in that case 2 *values* was sufficient.

We can see that, compared to MNIST, wotbot images have a huge potential *volume* (the number of pixels times `2^24`). Reducing them to the *substrate* *reduces* the *volume* (`40x40x10`) but now the wotbot has to choose the centre and the scale of the sample, so the action space goes from nothing to at least the number of pixels times the cardinality of a set of scales. In addition, there are many more *alignments* within the real images than there are within the MNIST handwritten digit images - a randomly chosen focus and scale in a real image is likely to view a random jumble of partially obscured objects, or areas of some object larger than the frame, or objects as if seen from a great distance. Even with an image that contains, say, a face that is reasonably centered and cropped, a set of similar faces slightly translated or seen at different distances is likely to yield lower *alignments* because there is no convolution. That is, the same object viewed with different fixation points will produce multiple *slices* and will dilute the *slice sizes* accordingly. Choosing exactly the same point of view for similar images will find a single *slice* much more quickly. This is where we can take advantage of the key idea of *likelihood*-accelerated *model* growth - by selecting the most *likely* foci for attention the active *inducer* can spend its time on the parts and resolutions of the images which will yield the greatest *alignments*. In general, the most *likely* focus will present the object so that it fits the frame nicely, but to some degree the exact choice will be somewhat arbitrary - so long as the same choice is used repeatedly, the *events* will be in the same *slice*. 

In the case of the turtlebot of `TBOT03` we search the *slice* topology to choose an action which we hope will transition to a high *likelihood* next *slice*. In the case of wotbot, there may well be a use later on for temporal *likelihoods*, especially in higher *levels*, but in `WBOT02` we will merely scan the current image to find the best place for the focus - the current *event* will be taken from the scanned frame with the highest *likelihood*. Of course, if the scene is changing rapidly, the new location may no longer be interesting, but it only need provide the starting location for the search in the scan of the next image, not the next *event* itself. If an interesting object is moving across the image slowly, wotbot should be able to track it until something else more interesting appears.

Although wotbot appears to be severely disadvantaged with respect to MNIST because of the need to saccade around the image, it does have an unlimited dataset to learn from - there are only 60,000 samples in MNIST. Active *models* are unlimited in size, so if the wotbot acts to use its compute resources efficiently by concentrating on interesting parts of the images, it has the potential to produce interesting *models*.

The wotbot [WBOT01 repository](https://github.com/caiks/WBOT01) was essentially a copy of the turtlebot [TBOT03 repository](https://github.com/caiks/TBOT03) without the turtlebot specific functionality. It depends on the [Robot Operating System](https://www.ros.org/), but for a robot that is missing locomotion and manipulation at this stage, ROS adds unnecessary complexity.

`WBOT02` is implemented instead with the [Qt](https://www.qt.io) framework. This allows for the same code to be used on multiple platforms and for cross-compilation to develop android apps. Although `WBOT02` does not really require the interface design tools in Qt Creator (because the GUI is relatively simple with little user interaction), Qt is useful in that it encapsulates the camera, video and screen-grab functionality in a single framework, which of course would otherwise require separate development for each platform. The library's [QImage](https://doc.qt.io/qt-6.2/qimage.html), [QPixmap](https://doc.qt.io/qt-6.2/qpixmap.html) and [QPainter](https://doc.qt.io/qt-6.2/qpainter.html) classes make the image manipulation straightforward. Note that the Qt thread functionality will not be used - [AlignmentActive repository](https://github.com/caiks/AlignmentActive) is based on [std::thread](https://en.cppreference.com/w/cpp/thread/thread).

## Sections

[Download, build and run in Windows](#main_Windows)

[Download, build and run in Ubuntu](#main_Ubuntu)

[Discussion](#Discussion)

<a name="main_Windows"></a>

## Download, build and run in Windows

This repository is based on the 6.2.4 version of [Qt](https://www.qt.io). The installation on Windows 11 is as follows. [Download](https://www.qt.io/download) the installer `C:\Qt\MaintenanceTool.exe`, and run it. Install Qt Creator and the `Desktop Qt 6.2.4 MSVC2019 64bit` kit. (Note that the default `Desktop Qt6.2.4 MinGW 64-bit` kit, which is GCC for Windows, may not have camera or screen grab functionality.) Check the various examples build and run ok.

Now install the [AlignmentActive repository](https://github.com/caiks/AlignmentActive), the [AlignmentRepaC repository](https://github.com/caiks/AlignmentRepaC) and the underlying repositories. Create a directory,

```
mkdir C:\caiks

```
Then use use [Github Desktop](https://desktop.github.com/), for example, to clone the following repositories to the `C:\caiks` directory -
```
https://github.com/Tencent/rapidjson.git
https://github.com/caiks/AlignmentC.git
https://github.com/caiks/AlignmentRepaC.git
https://github.com/caiks/AlignmentActive.git
```
Then clone the [WBOT02 repository](https://github.com/caiks/WBOT02) -
```
https://github.com/caiks/WBOT02.git
```
Then clone the [WBOT02 workspace repository](https://github.com/caiks/WBOT02_ws) -
```
https://github.com/caiks/WBOT02_ws.git
```
The WBOT02 executable can be built and run in Qt Creator by opening the `WBOT02` project in `C:\caiks\WBOT02` and building the Release configuration. Then in Qt Creator go to `Projects/WBOT02/Desktop_Qt_6_2_4_MSVC2019_64bit/Run` and set the `Command line arguments` to `hello`. Then run and check that `hello` appears in the `Application Output` window. To test `actor001` can grab the screen, set the `Command line arguments` to `actor001`. The `actor001` window should open on the left hand side of the screen and capture a rectangle on the right hand side of the screen.

The build can also be done from the command line, but in version 6.2.4 of Qt we must first set the necessary environment variables from Qt Creator. In Qt Creator go to `Projects/WBOT02/Desktop_Qt_6_2_4_MSVC2019_64bit/Build/Details` and hit `Open Terminal` to create a shell, then run `cmake` - 
```
"C:\Qt\Tools\CMake_64\bin\cmake.exe" -DCMAKE_PREFIX_PATH=C:\Qt\6.2.4\msvc2019_64 -S C:/caiks/WBOT02 -B C:/caiks/build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release

```
Then the executable can be built -
```
cd /d C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release
"C:\Qt\Tools\CMake_64\bin\cmake.exe" --build C:/caiks/build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release --target all

```
Finally the executable can be tested -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" hello > WBOT02.txt

```
Note that standard output does not echo to the shell but must be redirected to a file. Check that `C:\caiks\WBOT02_ws\WBOT02.txt` contains the word `hello`.

To test `actor001` can grab the screen, run the following -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001

```
The `actor001` window should open on the left hand side of the screen and capture a rectangle on the right hand side of the screen.


<a name="main_Ubuntu"></a>

## Download, build and run in Ubuntu

This repository is based on the 6.2.4 version of [Qt](https://www.qt.io). The installation on Ubuntu 22.04 is as follows:

First install the developer tools,
```
sudo apt update -y
sudo apt install -y git g++ cmake build-essential xorg libgl1-mesa-dev
sudo apt install -y qt6-base-dev qt6-base-dev-tools qt6-multimedia-dev

```
If running in the cloud, reboot the machine and reconnect, e.g. with `ssh -X`, and then test that the X server is running, e.g. with `xeyes`.

Now install the [AlignmentActive repository](https://github.com/caiks/AlignmentActive), the [AlignmentRepaC repository](https://github.com/caiks/AlignmentRepaC) and the underlying repositories, 
```
git clone https://github.com/Tencent/rapidjson.git
git clone https://github.com/caiks/AlignmentC.git
git clone https://github.com/caiks/AlignmentRepaC.git
git clone https://github.com/caiks/AlignmentActive.git

```
Then clone the [WBOT02 repository](https://github.com/caiks/WBOT02) and the [WBOT02 workspace repository](https://github.com/caiks/WBOT02_ws) -
```
git clone https://github.com/caiks/WBOT02.git
git clone https://github.com/caiks/WBOT02_ws.git

```
Then make the release build,
```
mkdir AlignmentC_build AlignmentRepaC_build AlignmentActive_build WBOT02_build
cd WBOT02_build
cmake -DCMAKE_BUILD_TYPE=RELEASE ../WBOT02
make
cd ~/WBOT02_ws
ln -s ../WBOT02_build/WBOT02 WBOT02

```
Then test the executable runs,
```
cd ~/WBOT02_ws
./WBOT02 hello

```
To test `actor001` can grab the screen, run the following,
```
cd ~/WBOT02_ws
./WBOT02 actor001

```
To obtain the YouTube videos discussed in connection with `actor003` below, install `yt-dlp`,
```
sudo add-apt-repository ppa:yt-dlp/stable
sudo apt update -y
sudo apt install -y yt-dlp  

```
To test, run the following,
```
yt-dlp https://www.youtube.com/watch?v=Y5dylh2aOiw

```
which should produce something like
```
[download] Destination: Amazing Physics Toys_Gadgets 1 [Y5dylh2aOiw].f303.webm
[download] 100% of 124.60MiB in 00:28
[download] Destination: Amazing Physics Toys_Gadgets 1 [Y5dylh2aOiw].f251.webm
[download] 100% of 7.84MiB in 00:01
[Merger] Merging formats into "Amazing Physics Toys_Gadgets 1 [Y5dylh2aOiw].webm"
```

<a name = "Discussion"></a>

## Discussion

Now let us investigate various wotbot modes and active structures. 

[Actor node](#Actor)

[Conclusion](#Conclusion)


<a name = "Actor"></a>

### Actor node

`WBOT02` has a `main` routine that examines the first command line argument and chooses its procedure accordingly. We discuss each of the procedures in the sections below -

#### screen001 and screen002

In `screen001` we test the screen grab functionality. We check to see how long it takes to (i) grab the pixmap, (ii) convert it to an image and (iii) read every pixel of the image. This is the typical output -
```
09:05:05: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe screen001...
auto pixmap = screen->grabWindow(0)	0.0549396s
auto image = pixmap.toImage()	6e-07s
image.format(): 4
image.depth(): 32
image.width(): 1920
image.height(): 1080
image.dotsPerMeterX(): 3780
image.dotsPerMeterY(): 3780
auto colour = image.pixel(QPoint(0,0))
colour: ff033db8
qAlpha(colour): 255
qRed(colour): 3
qGreen(colour): 61
qBlue(colour): 184
qGray(colour): 60
(qRed(colour)+qGreen(colour)+qBlue(colour))/3: 82
total: 1214020541
average per pixel:195	0.0014735s
09:05:05: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
On a Windows 11 laptop, the grab of the entire screen typically takes 30-50 ms, which limits the FPS to a maximum of 20. The processing of the image is much faster, around 1-2 ms.

In `screen002` we demonstrate that grabbing a rectangle rather than the entire screen is quicker. Here we open a Qt application, `Win002`. At fixed intervals the application grabs a rectangle, displays the captured imaged, calculates the average intensity, `(R + G + B)/3`, and produces some statistics -  

![screen002](images/screen002.png)

The command line is parsed as `screen002 <interval in ms> <x coord> <y coord> <width> <height>`. This example has a rectangle of `800x600` -
```
10:35:37: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe screen002 250 700 50 800 600...
application.exec(): pixmap.devicePixelRatio(): 1.25
captured	0.0598737s
image.format(): 4
image.depth(): 32
image.width(): 1000
image.height(): 750
image.dotsPerMeterX(): 3780
image.dotsPerMeterY(): 3780
auto colour = image.pixel(QPoint(0,0))
colour: ff034abf
qAlpha(colour): 255
qRed(colour): 3
qGreen(colour): 74
qBlue(colour): 191
qGray(colour): 67
(qRed(colour)+qGreen(colour)+qBlue(colour))/3: 89
average:220	0.0015486s
imaged	0.0063075s
captured	0.0337352s
average:212	0.0014052s
imaged	0.0023709s
captured	0.0215182s
average:212	0.00143s
imaged	0.0023704s
captured	0.0215584s
average:212	0.0011383s
imaged	0.0022505s
...
captured	0.0279223s
average:246	0.0008471s
imaged	0.0024594s
0
10:35:47: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
Capturing the smaller image of around a quarter of the area takes around half of the time, around 20 -25 ms.

#### camera001

`camera001` is similar to `screen002`, except that a camera image is captured instead of a rectangle of the screen. Also the process is asynchronous - the `capture` method of a `QImageCapture` is called and when the image is available the `imageCaptured` callback processes the image -

![camera001](images/camera001.png)

```
09:10:56: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe camera001 250...
QMediaDevices::videoInputs().count(): 1
application.exec(): capturing ...
captured	0.0100075s
image.format(): 4
image.depth(): 32
image.width(): 1280
image.height(): 720
image.dotsPerMeterX(): 3780
image.dotsPerMeterY(): 3780
auto colour = image.pixel(QPoint(0,0))
colour: ffa6a6a2
qAlpha(colour): 255
qRed(colour): 166
qGreen(colour): 166
qBlue(colour): 162
qGray(colour): 165
(qRed(colour)+qGreen(colour)+qBlue(colour))/3: 164
average:148	0.0012667s
imaged	0.0047368s
capturing ...
captured	0.023265s
average:147	0.0008s
imaged	0.0008467s
capturing ...
captured	0.0116363s
average:154	0.000817s
imaged	0.0007428s
capturing ...
captured	0.0165489s
average:161	0.0007532s
imaged	0.0007316s
...
capturing ...
captured	0.012549s
average:149	0.0007616s
imaged	0.0006978s
capturing ...
captured	0.0283564s
average:162	0.000711s
imaged	0.000911s
0
09:11:03: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
In this case the capture takes around 10 -30 ms.

#### video001 and video002

We made a couple of experiments with capturing frames from videos, `video001` and `video002`. The intention was to use the playlist of youtube videos from the [Kinetics dataset](https://www.deepmind.com/open-source/kinetics), which are a set of categorised videos of activities. Many of the videos, however, were filmed with quite a low resolution and so it is difficult to ensure that low scale frames are such that the cells are not all highly interpolated in any particular video. That is, many of the videos are very blurry at smaller scales. The capture process itself and automating a playlist were both difficult tasks too. For `actor001` and `actor002` we concentrated on grabbing videos from the screen and obtaining the videos from high resolution sources. Later on, in `actor003`, we successfully automated learning from a playlist of videos to allow headless *modelling* in the cloud.

#### Records and representations

Now let us consider scaled and centered average brightness records and their representations. The `Record` class is defined in `dev.h`. It represents a rectangular frame of a part or the whole of an image. It is defined by horizontal and vertical lengths or scales (fractions of 1) and a centre coordinate (a pair of fractions of 1). It consists of a two dimensional array of cells of integral value between 0 (dark) and 255 (light). It has persistence methods and a method to convert it to an *event*, i.e. a `HistoryRepa` of *size* 1 of a *substrate* consisting of (i) *variables* for each cell of a given cell *valency*, plus (ii) a scale *variable* of a given scale *valency*. 

`Record` has a constructor that creates a zeroed (black) array of cells. Another constructor creates a greyscale array of cells from a rectangular frame within a given image. Where an individual cell corresponds to a rectangle of pixels, an average pixel value is calculated. The value of the cell is the Hue-Saturation-Value calculation of lightness, i.e. the maximum of the red, green and blue values. 

The `valent` method reduces the *valency* of a record by sorting the *values* and calculating quantiles, with special handling of 0 (black) to deal with the case of a frame that overlaps with the boundaries of the given image. The `valentFixed` method reduces the *valency* of a record by simply dividing it into regular intervals.

The `image` method converts the record to an image with each cell generating a square of size `multiplier` pixels in the resultant image. The pixels are coloured grey with a lightness that depends on the cell's *value* as a proportion of the *valency*.

The following is a test of the `Record` persistence -
```
09:25:52: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe records...
rr
(0.9,0.8,0.7,0.7,3,2,[0,1,2,3,4,5])
(0.1,0.2,0.3,0.3,2,1,[6,7])

recordListsPersistent(rr, out)
rr2 = persistentsRecordList(in)
rr2
(0.9,0.8,0.7,0.7,3,2,[0,1,2,3,4,5])
(0.1,0.2,0.3,0.3,2,1,[6,7])

(0.9,0.8,0.7,0.7,3,2,[0,1,2,3,4,5])
(0.1,0.2,0.3,0.3,2,1,[6,7])

09:25:52: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```

The `Representation` class is also defined in `dev.h`. It represents the summation of a number of records, so it has a count of the number of records added, along with the array of integral cells. It also has an `image` method which converts the representation to an image with each cell generating a square of size `multiplier` pixels in the resultant image. This time the pixels are coloured grey with a lightness that depends on the average *value* as a proportion of the *valency*. So a representation can image a *slice* of *events*.

`Representation` also has persistence methods. These are needed to persist the map between *slices* and representations that is needed to visualise a *model*. The following is a test of the `Representation` persistence -
```
15:46:25: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe representations...
r1: (0.9,0.8,3,2,[0,1,2,3,4,5])
r2: (0.1,0.2,2,1,[6,7])
rr
(1,(0.9,0.8,3,2,[0,1,2,3,4,5]))
(2,(0.1,0.2,2,1,[6,7]))

sliceRepresentationUMapsPersistent(rr, out)
rr2 = persistentsSliceRepresentationUMap(in)
rr2
(1,(0.9,0.8,3,2,[0,1,2,3,4,5]))
(2,(0.1,0.2,2,1,[6,7]))

15:46:25: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```

#### screen003 and screen004

`screen003` is similar to `screen002` but adds records. The `screen003` capture procedure grabs the screen rectangle in the same way as `screen002`, but then creates records from the image. There is a record for each of 5 scales, which default to `[1.0, 0.5, 0.25, 0.125, 0.0625]`, all sharing the same centre. Each of these records has a *valency* of 256. For each of these greyscale records we create another bucketed record using the `valent` method, defaulting to a *valency* of 10. These two sets of records are converted to images with a `multiplier` of 3 and then arranged in two rows of descending scales below the grabbed image. The bottom row has the lower *valency* and these are what the wotbot will eventually see. Here is an example -

```
15:51:41: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe screen003 250 791 244 728 410...
application.exec(): 0
15:58:51: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```

![screen003_001](images/screen003_001.png)

The user can use the mouse to change the centre -

![screen003_002](images/screen003_002.png)

Notice that frames that exceed the grabbed image's boundary are filled in with black. This can be seen in the full scale image on the left.

`screen003` and `screen004` differ merely in how they are parameterised. Underneath they both use `Win005`. `screen003` expects its arguments on the command line, e.g.
```
screen003 250 791 244 728 410
```
`screen004` parses a JSON file in the manner of `TBOT03`, e.g.
```
screen004 actor.json
```
where actor.json is, for example,
```
{
	"interval" : 250,
	"scales" : [1.0, 0.5, 0.25, 0.125]
}
```

<a name = "actor001"></a>

#### actor001 description

`actor001` is the first version of the wotbot that uses the [active framework](https://github.com/caiks/AlignmentActive) to do dynamic *modelling*. Initially the focus is at fixed positions within the image; later we add a certain amount of randomisation. These experiments are similar to those of the random modes of [`TBOT03`](https://github.com/caiks/TBOT03#Random_modes_10_12).

`actor001` is implemented in `Win006`. The GUI is similar to `Win005`, except that there is now an additional row of images at the bottom. These are the representations of the *slices* to which the records of the second row belong. The *slices* *likelihoods* are shown below the *slice* representations. For example, `model001`, which has scales `[1.0, 0.5, 0.25, 0.125]`, produces the following -

![actor001_001](images/actor001_001.png)

The user can use the mouse to change the centre, or use the arrow keys (and space to return to the middle) -

![actor001_002](images/actor001_002.png)

In addition to the active, `Win006` has a *slice*-representation map which enables the third row of images. This map is persistent - when the window is destroyed the map is written along with the active.

The `Win006` constructor begins by parsing the given JSON file. Most of the parameters configure the active structure or logging; these are copied from turtlebot. The rest are specific to wotbot. In addition to the interval, screen grab, centre and scale list parameters of `screen004`, there is an offset list. These are relative offsets from the centre applied to each frame. We can then take multiple frames of the same scale from each act cycle image. In this example we have five quarter scale frames, one at the centre and the others arranged at the points of the compass -
```
{
...
	"scales" : [0.25, 0.25, 0.25, 0.25, 0.25],
	"offsets" : [[-0.125,0.0],[0.0,-0.125],[0.0,0.0],[0.0,0.125],[0.125,0.0]],
...
}
```

The constructor then creates the dynamic parts of the GUI. There are four `QLabel`s for each scale/offset - three are for the images of the *event* records and *slice* representations, and the fourth is for the *likelihood* statistic.

The constructor then creates the active structure. In this case, the structure consists of a single active with cumulative *slice size* and topology. The active has a single underlying `HistoryRepa`, which is updated by *events* generated from the frame records. That is, the *substrate* is just the record cells and a scale *variable*.

If `model_initial` is set, the *slice*-representation map and the active are loaded from their respective files.

Having constructed the actor a separate thread is started to run the active induce at regular intervals, if the `no_induce` flag is not set. Lastly, the constructor starts a `QTimer` to run the `Win006::act` callback.

The `Win006` destructor is called when the user closes the window. It writes the *slice*-representation map to a `.rep` file. It then signals the induce to terminate and waits for the induce thread to finish. It then dumps the active. After deleting the widgets it quits.

`Win006::act` is called at regular intervals according to the `interval` parameter. In the example *models* below, the interval is usually 40 ms, or 25 FPS. The act time is calculated and subtracted from the next pause so that there are regular intervals; the actors can then be considered continuous when we later come to consider temporal *levels* and *slice* topologies. If the act time exceeds the interval, the act is run again immediately with an optional warning. If the number of records taken (the length of the scale list) is four or more, i.e. over 100 records per second, then we often see act warnings and lagging induce, depending on the available compute. So, to prevent the induction lags, later versions of the actor will have to consider either having fewer *events* per second, or introducing discontinuities, i.e. having occasional large pauses between consecutive acts.

`Win006::act` first checks to see if the terminate flag is set. Then, if `event_maximum` if set, it checks to see if the event id exceeds it. In either case, it stops acting.

Next, `act` grabs the screen rectangle in the same way as defined in `Win005`. The records are then created from the image in the same way too, except that now we can optionally define a random offset which is applied to the centre before the frame offsets are added. In this case the parameterisation would be, for example,
```
{
...
	"scales" : [0.25, 0.25, 0.25, 0.25, 0.25],
	"offsets" : [[-0.125,0.0],[0.0,-0.125],[0.0,0.0],[0.0,0.125],[0.125,0.0]],
	"random_centreX" : 0.0625,
	"random_centreY" : 0.0625,
...
}
```
If the random offset is not set, the records are always taken from the same fixed points of the image. If we are grabbing images at the rate of 25 FPS and the action in the images is fairly static, then we can expect a lot of duplicated *events*. This will often make the *slice* representations identical or almost identical to frames taken at the same fixed locations when, say, we are browsing the *model* later on. Also, if the images are grabbed from a series of short videos with the same opening and closing sequences, the same *events* may well be additionally scaled by the number of videos in the series. If the random offset is set, however, the *events* are much less likely to be identical and so the *slice* representations are generally much more blurry and generalised, especially near the root of the *model*.

Next, an *event* is generated from each of the bucketed records and the active is updated. After the updates, the *likelihood* potentials of the *slices* of the new *events* are calculated from the *slice size* and the parent *slice size* according to this measure: `(ln(slice_size) - ln(parent_size) + ln(WMAX)) / ln(WMAX)`. Values close to one are highly expectedly *likely* or interesting. Values close to zero have low *likelihood*. Values around minus one or less are highly unexpectedly *likely* or unusual. With wotbot we will be mainly interested in high positive *likelihoods*. The induce threshold defaults to 200 and `WMAX` defaults to 18, so a zero *likelihood* implies around 11 *events* in the *slice*. A *likelihood* of 0.75 implies around 97 *events*. 

If the `no_induce` flag is not set, the representations of the *slices* of the new *events* are then obtained from the *slice*-representation map and the new *events* are added to them. The *model* is then checked to see if any new *fuds* have been added since the last act; if so, the representations of the new children *slices* are calculated. These two procedures should ensure that the leaf *slices* of the *model* always have the correct representations.

The remainder of `Win006::act` is as for `Win005::act` - the labels are updated with their latest images and statistics, and the `QTimer` set for the next act.

#### actor001 models

Now let us consider some of the *models* obtained by `actor001`. In order to make them comparable, we have trained them on the first few hours of the [Fireman Sam videos](https://www.bbc.co.uk/iplayer/episode/p08phyzv/fireman-sam-series-1-1-kite?seriesId=b00kr5w3). Fireman Sam is an animated character for children's television from the BBC. The programmes consists of around 51 episodes of about 10 minutes. In the early series the characters are animated plasticine; in later series CGI is used. The indoor scenes are generally fairly simple, in the early series at least, but outdoor scenes and later indoor scenes are more complex. The lighting is fairly even and primary colours are used a great deal - again, especially in the early series.

The default coordinates and rectangle for the screen grab may be checked by running with the following command line arguments and `actor.json` file -
```
actor001 actor.json
```
where actor.json is, for example,
```
{
	"interval" : 250,
	"scales" : [1.0, 0.5, 0.25, 0.125],
	"summary_active" : true
}
```
The browser's position should be arranged on the right hand side of the screen so that, for example, if the actor is started with the first film paused at 20 seconds, the capture looks like this - 

![actor001_003](images/actor001_003.png)

We can observe *induction* taking place in real-time by restarting the actor and then watching the representations in the third row. The four frames of different scales are recorded from a constant image centered at a single fixed point (0.5,0.5). So there are only four unique *events* during the whole session.

Initially, all *events* are in *slice* 0 and so there is no *likelihood*. The *slice* 0 representation is just an average of the four scales, showing part of the mirror on the left and a faint image of part of Sam on the right from the full scale frame. The `induceThresholdInitial` defaults to 1000 *events*. In this example there are 16 *events* per second, so the first *fud* is *induced* after around a minute, followed shortly after by two or three more. We observe the representations change as the new leaf *slices* are created for each *fud*. This is the screenshot afterwards -

![actor001_004](images/actor001_004.png)

We can see that the representations are now identical to the *event* records for each scale, and each has the same *likelihood* of 0.76. This is because the *model* is complete - each scale is now in a separate leaf *slice*. Although the *likelihood* is high, these leaf *slices* will not produce children *fuds* because all of their *histograms* are *singletons* which have no *alignments*. 

If we examine the logs we can see that the evolution of the *model* is not always exactly the same. In this example there are four *fuds* -
```
05:24:08: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe actor001 actor.json...
actor	status: started
application.exec(): model	induce summary	slice: 0	diagonal: 40.4438	fud cardinality: 1	model cardinality: 7	fuds per threshold: 0.189394
model	induce summary	slice: 131072	diagonal: 39.8886	fud cardinality: 2	model cardinality: 41	fuds per threshold: 0.359712
model	induce summary	slice: 131073	diagonal: 39.9336	fud cardinality: 3	model cardinality: 62	fuds per threshold: 0.520833
model	induce summary	slice: 131075	diagonal: 10.2801	fud cardinality: 4	model cardinality: 74	fuds per threshold: 0.671141
0
actor	status: finished
05:25:27: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
In this, there are only three *fuds* -
```
05:29:28: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe actor001 actor.json...
actor	status: started
application.exec(): model	induce summary	slice: 0	diagonal: 40.4874	fud cardinality: 1	model cardinality: 25	fuds per threshold: 0.185874
model	induce summary	slice: 131072	diagonal: 39.838	fud cardinality: 2	model cardinality: 32	fuds per threshold: 0.364964
model	induce summary	slice: 131073	diagonal: 39.875	fud cardinality: 3	model cardinality: 39	fuds per threshold: 0.535714
0
actor	status: finished
05:31:00: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
If we set the `induceThresholdInitial` to 200 and increase the `interval`, we can snapshot the intermediate *models*, 
```
{
	"interval" : 1000,
	"induceThresholdInitial" : 200,
	"scales" : [1.0, 0.5, 0.25, 0.125],
	"summary_active" : true
}
```
For example, after one *fud* the two largest scale frames and the two smallest scale frames have been separated into two *slices* -

![actor001_005](images/actor001_005.png)

Clearly the *induction* has correctly grouped the frames together in two pairs by similarity.

The *diagonal* of the first *fud* is a little lower (38.0 instead of 40.4) because of the smaller initial *slice* -
```
16:08:22: Starting C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe actor001 actor.json...
actor	status: started
application.exec(): model	induce summary	slice: 0	diagonal: 38.0308	fud cardinality: 1	model cardinality: 10	fuds per threshold: 0.980392
0
actor	status: finished
16:09:35: C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe exited with code 0
```
Now let us consider *models* created from first two hours of the Fireman Sam videos. In *model* 1 we will run with the same four scales centered at the same fixed point (0.5,0.5), and grabbed at a rate of 25 FPS. This is the definition of [`model001.json`](https://github.com/caiks/WBOT02_ws/blob/main/model001.json) -
```
{
	"model" : "model001",
	"interval" : 40,
	"scales" : [1.0, 0.5, 0.25, 0.125],
	"event_maximum" : 720000,
	"logging_action" : true,
	"logging_action_factor" : 1000,
	"warning_action" : true,
	"summary_active" : true
}
```
Start the films from the beginning of the first episode and run the following -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001 model001.json > model001.log

```
An example of the log is [model001.log](https://github.com/caiks/WBOT02_ws/blob/main/model001.log). The active will stop updating after 720,000 *events*, or approximately 2 hours. Note that the induce may be lagging, so wait until there are no more *fuds* being added by checking the log before quitting the application. When the application closes there are two files written to the `WBOT02_ws` directory - the active file, `model001.ac`, and the *slice*-representation map file, `model001.rep`. Both files are large, which is why we have not added them to the `WBOT02_ws` repository.

Now we can browse *model* 1 by setting the `no_induce` flag in `actor.json` -
```
{
	"model_initial" : "model001",
	"interval" : 1000,
	"scales" : [1.0, 0.5, 0.25, 0.125],
	"no_induce" : true,
	"logging_action_slice" : true
}
```
and then running with the following command line arguments -
```
actor001 actor.json
```
If we pause at 20 seconds of the first film as before, we see that the four representations are no longer exact copies of the records -

![actor001_006](images/actor001_006.png)

The first three representations are close to their corresponding records. All have high *likelihoods*. 

We can navigate around using the mouse or the arrow keys. The space bar will re-center the focus. We can easily navigate to areas of low *likelihoods*, for example if we hit the up arrow several times to (0.5,0.425) we have -

![actor001_007](images/actor001_007.png)

Of course, having a low *likelihood*, by the measure given above, does not mean that a *slice's fud* is *unlikely* - the representations still show the areas of light and dark to some degree - but only that the *slice* has perhaps less potential for future *model* than *on-diagonal slices*.

We may wonder why a small shift to the focus pushes us off the *diagonal*. The reason is probably that the place where we stopped is in the opening sequence of the Fireman Sam episodes, at least in the first series, so the *events* obtained are duplicated ten or so times. In addition, the scene pauses briefly while Sam looks at himself in the mirror, increasing the number of identical *events*. The frames that we have manually selected just a few pixels away from the fixed point of (0.5,0.5) are unlikely to have occurred at any other time during the two hours of running, and so their *slices* consist of just a few similar, but not identical, *events*. The smaller *counts* decrease the chances of the *slices* being on the *diagonal*. Although the scenes are only slightly translated, the *events* are completely different. This is because there is no notion of locality or convolution in the *substrate* - the 'distance' between any two *variables* has no meaning except to our eyes.

This reminds us of the vastness of the *volume* of the *substrate* compared to the compute resources available - we will have to manage the focus to go to exactly the same few hotspots relative to common scenes such as, say, faces or hands. We will also have to find the correct scale so that the object in question is always seen such that the *alignments* are maximised. That is, we must adjust for the  perspectival projection of a three dimensional object onto a plane. By scanning the image or searching the *slice* topology for the most *likely* foci and scales, we can accelerate *model* growth. The aim of the remaining `actor001` *models* is to examine the difference between fixed points of view and randomised points of view, for various scales, before we go on to consider *likelihood* based searches in `actor002`.

This table summarises the results from the `actor001` *models* -

model|scales|frame position|events|fuds|fuds/sz/thrshld|median diagonal|max diagonal|lagging fuds
---|---|---|---|---|---|---|---|---
model001|1.0, 0.5, 0.25, 0.125|centred|720,000|2817|0.7825|31.8063|41.4185|66
model005|1.0|1 centred|180,000|684|0.761591|33.4245|41.2544|0
model006|0.5|1 centred|180,000|697|0.775099|31.6855|41.1768|0
model007|0.25|1 centred|180,000|698|0.776233|31.458|40.8568|0
model003|0.5|1 centred, 4 offset|720,000|2762|0.767222|31.5374|41.7331|602
model004|0.5|1 centred, 4 offset, randomised|720,000|2951|0.819722|25.9151|41.3706|413
model002|0.25|1 centred, 4 offset|720,000|2807|0.779722|30.9492|41.6394|630
model008|0.25|1 centred, 4 offset, randomised|720,000|2730|0.758333|25.8088|38.3436|305

*Model* 1 is the only *model* with varying scales. In order to determine which scale had the most *alignments* we ran the first three scales separately, in *models* 5, 6 and 7. We can see that no scale has any noticeable advantage, with the *fuds* per *size* per threshold fairly constant amongst all of them at around 0.77. The median and maximum *diagonals* are similar too. 

The configuration for these three *models* is very similar. For example, `model005.json` -
```
{
	"model" : "model005",
	"interval" : 40,
	"scales" : [1.0],
	"event_maximum" : 180000,
	"logging_action" : true,
	"logging_action_factor" : 1000,
	"warning_action" : true,
	"summary_active" : true
}
```
And -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001 model005.json > model005.log

```
The remaining *models* have five frames, each with the same scale - a central frame and four frames at the points of the compass. In *models* 3 and 4 we compare the effect of fixed centres to randomised ones for half scale frames. In *models* 2 and 8 we do the same for quarter scale frames. We can see that for half scale the randomisation appears to increase the *model* a little (0.82 versus 0.77), but for quarter scale the randomisation appears to decrease the *model* a little (0.76 versus 0.78). There does not seem to be any very large quantitative differences.

Let us consider if there are qualitative differences between fixed and randomised for the half scale by comparing *models* 3 and 4. This is the configuration of `model003.json` -
```
{
	"model" : "model003",
	"interval" : 40,
	"scales" : [0.5, 0.5, 0.5, 0.5, 0.5],
	"offsets" : [[-0.25,0.0],[0.0,-0.25],[0.0,0.0],[0.0,0.25],[0.25,0.0]],
	"event_maximum" : 720000,
	"logging_action" : true,
	"logging_action_factor" : 1000,
	"warning_action" : true,
	"summary_active" : true
}
```
And -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001 model003.json > model003.log

```
We can browse the *model* by pausing the first film at 20 seconds and running `actor001 actor.json` with `actor.json` -
```
{
	"model_initial" : "model003",
	"interval" : 100,
	"scales" : [0.5],
	"no_induce" : true
}
```
At the centre we see an image very similar to that of the half scale frame from *model* 1, as we would expect -

![actor001_008](images/actor001_008.png)

If we display *model* 3 and *model* 1 side by side, we can easily see this -

![actor001_008](images/actor001_008.png) ![actor001_010](images/actor001_010.png) 

The *likelihoods* are both high as we would expect. 

If we move upwards by the same short distance as before to (0.5,0.425) we obtain the following -

![actor001_009](images/actor001_009.png) ![actor001_011](images/actor001_011.png) 

Here both *model* 3 and *model* 1 have low *likelihoods*. They are both far *off-diagonal* because they have seen few similar *events* at their fixed points.

The configuration of *model* 4 is very similar to *model* 3, but additionally has the random parameters; `model004.json` -
```
{
	"model" : "model004",
	"interval" : 40,
	"scales" : [0.5, 0.5, 0.5, 0.5, 0.5],
	"offsets" : [[-0.25,0.0],[0.0,-0.25],[0.0,0.0],[0.0,0.25],[0.25,0.0]],
	"random_centreX" : 0.125,
	"random_centreY" : 0.125,
	"event_maximum" : 720000,
	"logging_action" : true,
	"logging_action_factor" : 1000,
	"warning_action" : true,
	"summary_active" : true
}
```
And -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001 model004.json > model004.log

```
Now let us browse *model* 4 with `actor.json` -
```
{
	"model_initial" : "model004",
	"interval" : 100,
	"scales" : [0.5],
	"no_induce" : true
}
```
Let us compare all three *models* at (0.5,0.5) -

![actor001_012](images/actor001_012.png) ![actor001_008](images/actor001_008.png) ![actor001_010](images/actor001_010.png) 

The randomised case has a very low *likelihood* when compared to the fixed cases. This is as we would expect since there will be few, if any, identical *events* in the randomised case where the centre is not a fixed point.

Compare the randomised case to the fixed for the translated centre (0.5,0.425) -

![actor001_013](images/actor001_013.png) ![actor001_009](images/actor001_009.png) ![actor001_011](images/actor001_011.png) 

In this case it so happens that the randomised *likelihood* is very high. The representation is blurry, but the light and dark areas resemble the record fairly closely. Note that it is just chance that this happens to be a hotspot in the randomised *model* - if we navigate elsewhere, there are plenty of low  *likelihood* places nearby.

Neither the randomised nor the fixed methods perform very well where the frame is rarely seen. Let us see if frequently seen objects, such as Sam's face, improve things.

If we pause the first episode at 4 minutes 20 seconds, long after the opening sequence, *model* 4 does not have a hotspot at (0.5,0.5) -

![actor001_014](images/actor001_014.png) 

If we navigate around Sam's face with *model* 4 we find that all of the representations consist of blurry areas of light and dark when they have high *likelihoods*. For example,

![actor001_015](images/actor001_015.png) 

None of the representations look very much like his face. This suggests that randomised *modelling* would need much longer runs to attain, say, two darker areas for the pupils of the eyes. A limited active *history* is likely to preclude ever obtaining even a basic *model* of faces, unless special methods are used to accelerate *model* growth.

If we repeat for *model* 3 we have a high *likelihood* hotspot at (0.5,0.5) -

![actor001_016](images/actor001_016.png) 

The representation is possibly less blurry than for the *model* 4 hotspot, but still does not look like a face.

We can perhaps tentatively conclude that frequently seen frames are more likely to have high *likelihood* hotspots nearby, in varying degrees of *decomposition*, than frames containing the intervals between objects (such as the edge of the mirror above), or with jumbles of rarely seen objects. To find out, `actor002` will actively scan for hotspots and will show the siblings and ancestor *slice* representations. 

None of the *models* with only one frame (5, 6 and 7) have induce lags. There is a small lag in *model* 1 with four frames, and large lags in *models* 3, 4, 2 and 8 with five frames. The *models* that lagged the most may have slightly better *models* because of the larger *slices*. The lagging is mainly a problem, however, because the *model* is out of date, and so it will be important to avoid lags when we come to *likelihood* search in `actor002`. Eventually  in `actor003` we will run the image capture from video, and the active update and induce will be done synchronously, thus avoiding lags altogether.

Note that the `actor001` *models* were all subject to a bug in the record `valent` bucketing which meant that the first *value* was too infrequent and the last *value* was too frequent. Thus the *valency* was more like 9 than 10.  Qualitatively, however, the *models* still seem to work quite well. The `actor002` *models* were all corrected nonetheless.

<!-- TODO 

For model 8 there are 20/28 hotspots > 0.5 every 20s of the first episode at (0.5,0.5) and 10/28 in episode 2. At (0.562,0.562) there are 14/28 hotspots and 10/28 in episode 2. So pretty random. Do the same for fixed - model 2 at (0.5,0.5) has 19/28 episode 1 and 18/28 episode 2. So there probably is an advantage for the fixed. However, even for random there is at least a 50% chance a location is a hotspot.

Scenes from the credits have nearly uniformly high likelihoods (> 0.5) and the slice is usually right. Scenes from the first series have variable results. The slices are wrong quite frequently, although perhaps show a scene which is one of the stock backgrounds but does not have the object in the foreground, such as a bus moving along a road. Scenes from other series usually have low likelihoods and often the slices are only approximate or completely wrong.  

don't seem to be able to show anything convincing by browsing, but the likelihoods of the randomised model around faces seem to be higher than in non-decript areas in general e.g. FS1_1_1m40s_008_1.png

cf CAIKS4 202205310940

-->

#### actor002 and actor003 description

In `actor001` the fixed position *models* had many identical *events* in a *slice* from the opening introduction sequence, the closing credit sequence, and other pauses in the action. Some *slices* might even be identical to a single *event*. We would like to compare these *models* to those created in random or other modes. To see what is going on in a *model*, the `actor002` GUI displays the representations and *likelihoods* of the ancestors and siblings of the current centre's *slice*. It can also optionally display some example *events* from the *slice*.

This ability to 'browse' the *model* enables us to make qualitative estimates of how generic the *model* is. Of course we do not want it to be too generic - the *model* would never become large enough to be useful in the compute time available without a solution to the problem of *model* duplication due to translation. So we need to explore modes that focus the search on hot spots and hot scales. These are rather like the fixed points, in that the *slices* will be more specialised, but also like the random points because the hotspots initially have a certain degree of arbitrariness about them.

`actor002` was developed to both create *models* and browse them, but the event loop architecture of a GUI means that the callbacks must remain short and so the active induce must be done in a separate thread. This can cause the *model* to lag behind the *events* depending on the act mode and the active induce parameterisation. Dealing with the lags and interruptions tends to make it difficult to reproduce experiments - especially as the *event* capture is by screen grab. In addition, occasional prompts to continue from the BBC website used for the Fireman Sam videos meant that a motion detector was required to pause the active when the action stops. The Windows O/S that was used for development was a desktop O/S rather than a server O/S so there there were problems arising from the operating system's scheduling of long running processes. Ultimately wotbot will comprise a client-server architecture with the sensors and actuators on the client, e.g. a mobile phone, and the active system running on a server machine. 
 
To that end `actor003` dispenses with the interactive *model* browser parts of the GUI. In fact, it can be run without a GUI at all, if monitoring is not needed. The *models* generated by `actor003` can be browsed in `actor002` because the active structure is the same. Also, `actor003` captures the *events* from a list of videos on the local filesystem, rather than requiring a user to start and stop the streaming videos in the grabbed screen area. The capture, act, update and induce all are designed to run synchronously, i.e. the media is paused so that the captures are regular regardless of the induce time. (Although, depending on whether the underlying video library can seek within a video, Qt sometimes runs the media player ayschronously in a separate thread. In that case the intervals between *events* will be less regular. Even so, we can expect the *model* to be more reproducible with this method.)  Also, `actor003` can run on a server O/S and so the compute intensive scanning modes described below can be implemented on a much larger machine than is typical for a desktop.

`actor002` uses `actor001` as a template and much of it has not changed. `actor002` is implemented in `Win007`.  As in `actor001`, the GUI shows the captured image in the top half of the window. Depending on mode, it overlays bounding boxes around the areas of the records selected for active update. 
 
Note that in the examples below we often use mode 4. In this mode we scan the area within the largest box (in grey) and choose the frame with the longest *slice* path (which is also called 'actual *likelihood*'). This frame, highlighted in white, is where the centre is set. The other frames shown, if any, are the frames with the next longest *slice* paths. This mode, which will be described in detail in the discussion of *models* below, snaps the focus to the best hotspot within the scan area and so it generally produces a *slice* with many ancestors and in which the examples are very similar.
 
Also, many of the images are from *models* generated in `actor003` which relies on a list of videos. The videos we used were a selection of black and white videos downloaded from YouTube. In general we used Film Noir videos. These have many scenes of characters talking, and so faces, heads and shoulders are very frequent. Note that the actors are usually white which means that  *models* that depend on brightness and not edge detection will perform poorly in cases where the actors have a more diverse range of ethnicities. 
 
The bottom part of the GUI is now given over to browsing. To enable it, set `interactive` in the configuration. For example, run the following -

```
actor002 actor.json

```
with `actor.json` -

```
{
	"interval" : 250,
	"x" : 870,
	"width" : 560,
	"model_initial" : "model010",
	"mode" : "mode004",
	"event_size" : 5,
	"threads" : 6,
	"valency_factor" : 3,
	"scale" : 0.177,
	"range_centreX" : 0.236,
	"range_centreY" :0.177,
	"interactive" : true,
	"disable_update" : true,
	"summary_active" : false,
	"logging_action" : false,
	"warning_action" : false
}
```
Here is an example from Fireman Sam captured in the opening sequence of the first episode -
 
![actor002_model010_Sam_001](images/actor002_model010_Sam_001.png) 

Now `actor002` differs from `actor001`; instead of the array of representations for all of the scales that we see in `actor001`, we are able to see more details of the position within the *model* of the current *slice*. 

The first row shows, from right to left, (i) the greyscale record of the frame at the current centre and scale (highlighted in a white bounding box above), (ii) the bucketed or regular interval *valency* record, and (iii) the representation and *likelihood* of the record's *slice*. In general, depending on the length of its *model* path and its *likelihood*, the representation of the *slice* will resemble the greyscale record in terms of its light and dark regions. 

In the second row the sibling *slice* representations and their *likelihoods* are shown in descending order of *likelihood* or, equivalently, *slice size*. If there are too many siblings, the number of missing siblings is shown at the right. Note that the current *slice* is not necessarily the most *likely*  of the siblings, so it might not be the first in the sequence. Note also, that there are usually two or more siblings that are *on-diagonal* (i.e. with high *likelihoods*), with the others far *off-diagonal*  (i.e. with low *likelihoods*). If the mode is scanning for potential *likelihood*, the *slice* will usually be at the beginning of the siblings. 

The third row shows the ancestor *slices*, with the current *slice* shown first, then its parent, grandparent, great-grandparent, et cetera to the root *slice*. If there are too many ancestors the number of missing ancestors is shown at the right.  If all of the ancestors are visible, the sequence shows from right to left the increasing specialisation of the *slice* along its path through the *model*, from the most general at the root, which is the average of all *events*, to the *slice* itself at the leaf.
 
To show a selection of example *events* from the current *slice*, set `interactive_examples` in the configuration. (We can also set `multiplier` to 1 to show smaller images of the example *events*.) For example, `actor.json` -
```
{
	"interval" : 250,
	"x" : 870,
	"width" : 560,
	"model_initial" : "model010",
	"mode" : "mode004",
	"event_size" : 5,
	"threads" : 6,
	"valency_factor" : 3,
	"scale" : 0.177,
	"range_centreX" : 0.236,
	"range_centreY" :0.177,
	"interactive" : true,
	"interactive_examples" : true,
	"multiplier" : 1,
	"label_size" : 16,
	"disable_update" : true,
	"summary_active" : false,
	"logging_action" : false,
	"warning_action" : false
}
```
![actor002_model010_Sam_002](images/actor002_model010_Sam_002.png) 

The examples row is inserted between the top row and the siblings row. There are as many example representations as can be fitted in the available images (defined by `label_size` in the configuration). If the *slice* has more *events* than the available images, they are taken at regular intervals from the *slice*.

In the Fireman Sam example above we can see the *slice* matches the greyscale record only very roughly. The bucketing of the *valency* record has exaggerated the slightly varying brightness of the wallpaper to the right of the mirror frame, and so the *slice* has matched a dark area in the top right which is really just an artefact of the `valent` algorithm. 

There are two *on-diagonal* siblings in this case, with the current *slice* being the larger of the two. *On-diagonal* siblings usually resemble each other except for where the *derived alignment* of their *fud* forks the divergence. The *on-diagonal* siblings are usually still very generalised, depending on the length of their path, while the *off-diagonal* siblings look like the ghostly mixtures of the few *events* they contain.

In the Fireman Sam example above we can see that the examples vary a lot, but they all have darker areas on the left hand margin and at the top right. None of them look like mirror frames.
 
We can examine the same image with a *model* that uses the `valentFixed` algorithm instead. For example, `actor.json` -
```
{
	"interval" : 250,
	"x" : 870,
	"width" : 560,
	"model_initial" : "model036",
	"mode" : "mode004",
	"event_size" : 1,
	"threads" : 6,
	"valency_fixed" : true,
	"scale" : 0.177,
	"range_centreX" : 0.04425,
	"range_centreY" :0.04425,
	"interactive" : true,
	"interactive_examples" : true,
	"multiplier" : 1,
	"label_size" : 16,
	"disable_update" : true,
	"summary_active" : false,
	"logging_action" : false,
	"warning_action" : false
}
```
![actor002_model010_Sam_003](images/actor002_model010_Sam_003.png) 

We introduced the `valentFixed` algorithm when we switched from Fireman Sam to Film Noir (see *model* 25 and after, below). We can see that the examples above are from the Film Noir films rather than Fireman Sam, but the *slice* has matched the darker area in the top left correctly.

In this scene from 'Citizen Kane' the mode 4 focus has found a hotspot just above the woman's left eye using *model* 41 -

![actor002_model041_Film_Noir_014](images/actor002_model041_Film_Noir_014.png) 

Some of the examples also look like foreheads just above an eye, and the *slice* path length is 20, so the classification is quite specialised and finds a lot of similar images.

The fixed intervals of the `valentFixed` algorithm produce better results with Film Noir videos because of the high contrast used by the cinematographers. The bucketing of the `valent` method tended to magnify small variations in low contrast areas of background or textures. This caused the *modelling* to be distracted away from interesting foreground objects.

The `valentFixed` algorithm was a step forward but we found that the *models* were still tending to be very interested in the infinite variations of black that are a feature of Film Noir. Later *models* (47 onward) set a minimum value for the entropy of the record in order to exclude slightly varying areas of background, especially dark corners of interiors or light areas of sky. In order to judge what entropy level to use we can show the entropy of the record in the GUI by setting `interactive_entropies` in the configuration, e.g.

![actor002_model050_Film_Noir_003](images/actor002_model050_Film_Noir_003.png) 

Here we have added the entropy below the second and third records in the top row, and below each of the example records in the second row. The entropies are not to be confused with the *likelihoods* underneath the representations elsewhere. In this example, the entropies are all quite high. The highest is grey-scale record (third in the top row) at 4.188. This is as expected because of its *valency* of 256, which is much greater than the 10-*valent* *substrate*. The entropies also help us distinguish between very similar but not identical *events*, such as the five examples on the right.


<!-- TODO 

Do actor 2 for likelihood. Only need one current frame visible for a single active. Idea is to experiment with different scans. The displayed frame under user control need not be an event, only part of the scan.

Actor 2 GUI will have current slice, 10 valent actual and 256 actual and colour actual in the first row. The top 5 siblings are in the next row, excluding the current, in descending order of likelihood. The likelihoods are shown. The last 5 ancestors are shown in the third row along with likelihoods. Statistics of the model and timings below that.

What are the siblings for the slices that seem to be images? Are they also the same image but translated? Only slightly e.g. alarm clock for model 9 - not translated but ringing differently, or at 1:03 of Sam. Show the relations nearest a browsed slice. Perhaps a decomp browser.

-->

#### actor002 and actor003 models

This summarises the *model* results for both `actor002` and `actor003` -

model|scales|mode|mode id|valency|domain|events|fuds|fuds/evts/ths (at 1m)|mean length|std dev length|max length|fails|notes
---|---|---|---|---|---|---|---|---|---|---|---|---|---
model010|0.5|4 randomised|1|bucketed|Fireman Sam|720,000|2,567|0.713|6.82|1.83|12||25 FPS
model011|0.354|4 randomised|1|bucketed|Fireman Sam|720,000|2,699|0.750|7.22|1.91|12||
model012|0.25|4 randomised|1|bucketed|Fireman Sam|720,000|2,664|0.740|7.23|38.2333|12||
model014|0.25|4 randomised|1|bucketed|Fireman Sam|720,000|492|0.683|5.40|1.61|9||1000 threshold
model013|0.177|4 randomised|1|bucketed|Fireman Sam|720,000|2,719|0.755|7.19|1.91|13||
model015|0.177|4 potential|2|bucketed|Fireman Sam|720,000|4,131|1.148|9.49|1.72|14||20 randomised
model016|0.177|4 actual-potential|3|bucketed|Fireman Sam|720,000|3,014|0.838|10.13|1.87|14||20 randomised
model018|0.177|4 actual-potential|3|bucketed|Fireman Sam|1,000,000|4,194|0.839|10.48|1.88|15||20 randomised
model017|0.177|10 scanned actual-potential|3,4|bucketed|Fireman Sam|1,000,000|4,397|0.880|11.58|2.83|20||4 FPS
model019|0.177|4 potential|2|bucketed|Fireman Sam|1,000,000|5,738|1.148|9.81|1.76|15||20 randomised
model020|0.177|10 scanned actual-potential|3,4|bucketed|Fireman Sam|1,000,000|5,526|1.106|10.76|2.75|19|
model021|0.177|10 scanned actual-potential|4|bucketed|Fireman Sam|720,000|3,389|0.942|13.83|2.83|21|
model022|0.177|10 scanned actual-potential|4|bucketed|Fireman Sam|1,000,000|4,729|0.946|14.41|2.87|22|
model023|0.177|10 scanned actual-potential|4|bucketed|Fireman Sam|1,300,000|5,488|0.845 (0.946)|14.66|2.89|22|
model024|0.177|10 scanned actual-potential|4|bucketed|Fireman Sam|1,700,000|6,172|0.726 (0.946)|14.84|2.90|22|
model025|0.177|10 scanned actual-potential|4|fixed|Film Noir|865,170|5,283|1.221|18.73|7.35|50|179|
model027|0.177|10 scanned actual-potential|4|bucketed|Film Noir|759,760|3,757|0.989|14.14|3.13|24|12|
model028|0.177|10 scanned actual-potential|4|bucketed|Film Noir|659,550|3,203|0.971|14.84|3.14|27|49|
model029|0.177|5 scanned potential tiled actual-potential|5|bucketed|Film Noir|526,345|3,386|1.287|13.49|2.60|23|2|
model030|0.177|5 scanned potential tiled actual-potential|5|fixed|Film Noir|527,045|3,816|1.448|15.56|3.74|31|27|
model031|0.177|5 scanned potential tiled actual-potential|5|fixed|Film Noir|452,255|3,197|1.414|14.62|3.73|32|31|12.0 min diagonal
model034|0.177|5 scanned potential tiled actual-potential|5|fixed|10 B&W videos|527,045|3,846|1.459|15.17|4.40|35|21|first actor003 model
model035|0.177|5 scanned potential tiled actual-potential|5|fixed|14 B&W videos|1,000,000|7,069|1.414|15.92|3.50|35|16|
model036|0.177|4 randomised|1|fixed|12 B&W videos|500,000|1,931|0.772|7.20|2.00|17|6|
model037|0.177|5 scanned potential tiled actual-potential|1,5|fixed|12 B&W videos|1,000,000|6,033|1.207|12.60|5.11|35|60|two modes at 7 and 15
model038|0.177|4 potential|2|fixed|12 B&W videos|500,000|2,488|0.995|9.04|2.12|18|7|20 randomised
model039|0.177|5 scanned potential tiled actual-potential|5|fixed|12 B&W videos|1,000,000|7,214|1.443|16.42|5.40|46|43|30s unique
model040|0.177|5 scanned potential tiled actual-potential|5|fixed|12 B&W videos|2,000,000|12,262|1.226 (1.443)|17.77|6.01|54|72|30s unique
model041|0.177|5 scanned potential tiled actual-potential|5|fixed|12 B&W videos|2,656,962|14,079|1.060 (1.443)|18.30|6.61|62|102|30s unique
model045|0.177|5 scanned potential tiled actual-potential|5|fixed 5-valent|48 B&W videos|2,656,963|34,557|1.300 (1.643)|20.2|5.09|44|297|30s unique, 12.0 min diagonal, 100 threshold
model046|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|2,656,965|13,432|1.011 (1.406)|16.8|4.49|36|67|30s unique, 12.0 min diagonal
model047|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|2,656,963|13,688|1.030 (1.364)|15.9|3.07|25|0|30s unique, 12.0 min diagonal, 1.2 min entropy
model048|0.177|5 scanned potential tiled actual-potential|5|fixed 5-valent|48 B&W videos|2,656,965|38,124|1.434|18.9|3.43|30|0|30s unique, 12.0 min diagonal, 100 threshold, 0.8 min entropy, no overflow
model049|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|2,656,964|36,146|1.360|16.7|2.87|28|0|30s unique, 12.0 min diagonal, 100 threshold, 1.2 min entropy, no overflow
model050|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|2,656,962|16,980|1.278|15.3|2.78|23|0|30s unique, 12.0 min diagonal, 1.2 min entropy, no overflow
model051|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|2,656,963|8,079|1.216|15.1|2.94|23|0|30s unique, 12.0 min diagonal, 400 threshold, 1.2 min entropy, no overflow
model052|0.177|4 randomised|1|fixed|48 B&W videos|500,000|1,887|0.753|6.7|1.64|11|0|30s unique, 12.0 min diagonal, 1.2 min entropy, no overflow
model053|0.177|4 randomised|1|fixed|48 B&W videos|500,000|1,910|0.764|6.6|1.58|11|0|30s unique, 12.0 min diagonal, 1.2 min entropy, no overflow
model054|0.177|5 scanned potential tiled actual-potential|5|fixed|48 B&W videos|3,000,065|12,390|0.826 (1.294)|15.5|2.90|24|0|30s unique, 12.0 min diagonal, 1.2 min entropy
model055|0.177|5 scanned size-potential tiled actual-potential|6|fixed|48 B&W videos|3,000,000|15,050|1.003 (1.489)|14.8|2.92|24|0|30s unique, 12.0 min diagonal, 1.2 min entropy
model056|0.177|5 scanned size-potential tiled actual-potential|6|fixed 5-valent|48 B&W videos|3,000,000|34,859|1.162 (1.643)|19.7|4.02|32|0|30s unique, 12.0 min diagonal, 1.2 min entropy

The median diagonals for the actor 2 *models* were consistently in the range 23-27, and the maximum diagonals were consistently in the the range 37-39.

<!-- TODO 

Noticeable difference between the fixed and randomised, probably because the randomised is now uniformly random rather than around 5 fixed events. Also the long lags in actor 1 meant that the slices were larger and would be better modelled. 

The `0.354 = 1/2^1.5` scale seems to do better. And 0.25 is better still. `0.177 = 1/2^2.5` is the highest, but is getting down to the resolution of the animation.

Model 23 fuds per events per threshold = 0.845, so has declined from 0.946. If had continued at original rate would have been 6137 fuds, i.e. running at about half the growth rate, at 0.506 fuds/sz/thrshld or 53% of non-overflow rate. The path lengths have not changed much, not half the diff between model 720k (model 17) and 1000k (model 22). So overflow has a large effect. Model 24 growth has declined again, at 0.342 fuds/sz/thrshld or only 36% of non-overflow rate. But note that later series of fireman Sam are CGI and have more colours but perhaps less contrast so often the focus is on the backgrounds rather than the characters or objects.

Compare non likely models esp random versus fixed. Use screenshots comparing fixed and random for repeated scenes, learned scenes and new scenes. Draw conclusions regarding scanning and likelihood search. Compare the random models to the fixed models for large likelihoods around faces, etc. Expect the likely models > fixed models > random models in hot spots near faces in credits, but likely models > random models > fixed models otherwise. Key thing is to position the focus on the hot spots (and hot scales) in order to avoid the need for convolving models over many positions. Can we conclude anything about likelihoods from navigating manually? don't seem to be able to show anything convincing by browsing, but the likelihoods of the randomised model around faces seem to be higher than in non-decript areas in general e.g. FS1_1_1m40s_008_1.png. Could do a likelihood map of an image by scanning and colouring each pixel according to its likelihood, but manual navigation suggests that this would be a very jagged landscape - which poses a question mark over the golf ball approach to scanning. Is a gentle pressure enough to demonstrate intelligence - an ineffable quality which is considerably harder to show than faster model growth?

contour discussion. do the generate_representation to see if there are noticeable qualitative differences between model 21 and 24 . contour comparisons. Compared to model 16 model 21 has very concentrated hotspots. If we manually move around nearby the model has very short paths.

evidence of interesting features. Browsing around model 27 with a small scan area suggests that the sky will still be interesting in mode 5, but of course filmakers do not focus on the sky very much, so much of the image will be interesting anyway and there is no bias for backgrounds particularly.

Statistics. I think it will be difficult to obtain a good measure of model quality. The higher moments of model 17 (scanned act-pot) are tending to more normal rather than fat RHS tails, which was the intuition. The actor will always be scanning, so much of the model is redundant. We can see this if we run model 17 in mode 4 without update - the slice's path is usually well over the mean length and often at the maximum length (18). But how do we get this 'typical' slice subset. Probably the main statistics are the maximum length and the deviation. Not much difference in the stats between random pot and random act-pot, so try running scanned act-pot over random pot, to see what might be a good fast initial model type.

Non likely versus likely modes. Golf ball approach does not work because there is no smooth gradient. Must scan.

Film Noir and fixed buckets

decent demo of scanning

the potential and actual-potential models demonstrate that the search for model growth can be achieved even without restricting the history size, which we were forced to do in TBOT03.

evidence of interesting features. The examples of interactive model 41 in `C:\caiks\WBOT02\images` suggest that lighting is very important as well as scale - so edge detection would be interesting. There are many faces in the examples, but it is rare that a frame is classified with a lot of examples that we would recognise as similar, e.g. actor002_model041_Film_Noir_010.png. Even then, there are many examples which are completely unrelated to faces or eyes. It is clear that the classification is good on broad areas of light and dark, but the model would have to be much larger I think before the facial features would be reliably classified together and even then they would probably be spread over slices that are far apart in the model because of lighting conditions. We can see that scale is very important too - compare actor002_model041_camera_001.png and actor002_model041_camera_002.png, where the images magnification is slightly different.

The very long paths are usually dark black, but there are plenty of hotspots around medium close up faces that are longer than the first mode of 17. There are lots of surfaces and body edges that have longer paths. (Increase the diagonal?) The face slices usually consist of many other non-face events, although it is hard to tell from the off-diagonal siblings. Essentially, the model is recognising faces, but they are buried amongst a lot of other alignments. The lighting of the image is important too - the siblings all have the same brightness. So we must consider edge detection and instantanous motion after multi-scale.

Duplicate removal. Can prevent exact duplicates if we use the motion detector hash for each event and then check the event against the current slice or the most recent slices. Or simply check against the list of the most recent events, so that duplicates are allowed after a suitable interval. Prefer to check the recent slices, but could use a long list of events. Events is easier - use a FIFO queue and a set. Could process before choosing topmost of the mode. First do the browser and get evidence that it is a problem. Duplicate removal. Surely the whole point of centering is to get a lot of similar frames quickly? Similar - but not the same. Removing duplicates might allow unusual events to get more attention. Count the duplicates.

Scanning. What we really want is local actual-potential and global potential. The actual-potential scan is really to center and scale the frame and so avoid duplicating model because of affine transformations. But we want global potential to maximise the modelling rate. Instead of mode 5 random-topology we could systematic tiling of local scans. That is, a mode 4 variation where we sort actual-potential for each tile and then choose the top tiles by potential. This would have all of the current performance problems, however, and does not allow for scaling. Of course, we did accidentally run model 27 without the valency_factor and this would be an easy change that would improve the modelling rate. There are a lot of parameters associated with random-topology searches too.

"We were able to demonstrate that deliberately searching for interesting or unusual slices, ... can accelerate model growth, at least in the case of limited active history size." Now with WBOT02 we have proved it without constraining size, although the search is a scan and then selection of likely centres.

"wotbot should be able to track it until something else more interesting appears" - put an example of a video and model of the wotbot tracking successfully. Use a model from actor 3, since more reproducible.

The models are still very blurry and so we must try to increase model detail but concentrated in areas interesting to us humans. These are not that much more interesting than clouds and backgrounds. With multi-scale, face hotspots will be more common and that might accelerate model growth there. Also if we have dynamics, the backgrounds will be more separated from heads and bodies - although that will geneally be the case anyway. Once attached to a head the small scan hotspot mode tends to follow it around - so there is a chance that we can parameterise the mode so that the model can be interested in the objects we are too.

model 46 -
```
lengthsDist: {(1,1),(2,17),(3,18),(4,31),(5,100),(6,228),(7,375),(8,661),(9,1205),(10,1973),(11,3334),(12,5220),(13,7381),(14,9115),(15,10368),(16,10740),(17,9662),(18,8370),(19,6331),(20,4626),(21,3394),(22,2374),(23,1874),(24,1811),(25,1556),(26,1399),(27,1106),(28,830),(29,652),(30,437),(31,280),(32,213),(33,106),(34,77),(35,54),(36,19)}
lengthsCount: 95938
lengthsMean: 16.8148
lengthsDeviation: 4.48833
lengthsSkewness: 0.768545
lengthsKurtosisExcess: 1.05939
lengthsHyperSkewness: 7.36744
```
model 47 -
```
lengthsDist: {(1,3),(2,12),(3,20),(4,60),(5,120),(6,236),(7,417),(8,839),(9,1317),(10,2339),(11,3604),(12,5775),(13,8134),(14,10667),(15,13343),(16,14043),(17,13979),(18,12387),(19,9497),(20,6270),(21,3264),(22,1587),(23,503),(24,169),(25,12)}
lengthsCount: 108597
lengthsMean: 15.856
lengthsDeviation: 3.06973
lengthsSkewness: -0.376717
lengthsKurtosisExcess: 0.249775
lengthsHyperSkewness: -3.94355
```

Noticeably different statistics between models without and with min entropy. Same mode, but lower deviation and max length and sign of skew has changed and smaller kurtosis - all suggest a more balanced stubbier tree, with more shorter paths (presumably lowish enrtropy/alignments) and fewer longer paths (but perhaps more interesting to us). No fails also suggests that all of the failed slices in the previous model where low entropy. All but 12 of the 1556 slices of length 25 or more were presumably also low entropy.

If model 47 had no overflow, it probably would have had 18,120 fuds at 1.364 fuds/sz/200. Compare this to 5-valent model 48 which had 38,124 fuds at 1.434 fuds/sz/100, or 19,062 equivalently. That is around double. If we assume that min entropies are equivalent, we can conclude that the valency does not have much effect on growth. This seems consistent with the diagonals.

model 47 -
```
lengthsDist: {(1,3),(2,12),(3,20),(4,60),(5,120),(6,236),(7,417),(8,839),(9,1317),(10,2339),(11,3604),(12,5775),(13,8134),(14,10667),(15,13343),(16,14043),(17,13979),(18,12387),(19,9497),(20,6270),(21,3264),(22,1587),(23,503),(24,169),(25,12)}
lengthsCount: 108597
lengthsMean: 15.856
lengthsDeviation: 3.06973
lengthsSkewness: -0.376717
lengthsKurtosisExcess: 0.249775
lengthsHyperSkewness: -3.94355
```

model 48 -
```
lengthsDist: {(1,2),(2,3),(3,12),(4,32),(5,49),(6,101),(7,190),(8,358),(9,633),(10,1105),(11,1933),(12,2950),(13,4553),(14,6566),(15,9351),(16,12512),(17,15831),(18,18657),(19,21069),(20,21341),(21,19216),(22,15614),(23,11678),(24,7486),(25,3789),(26,1728),(27,599),(28,121),(29,52),(30,10)}
lengthsCount: 177541
lengthsMean: 18.9173
lengthsDeviation: 3.43071
lengthsSkewness: -0.418654
lengthsKurtosisExcess: 0.262438
lengthsHyperSkewness: -4.25479
```
In spite of being twice the model, the leaf slices only increase from 108,597 to 177,541 slices, ie only 60% more. This is because the increase in model was not evenly distributed but concentrated, presumably on the hotspots.

Mode increases from 16 to 20 and the max from 25 to 30, and the mean from 15.9 to 18.9, so the statistics seem to similar as expected.

Model 49 is definitely less likely to focues on bodies and faces than model 48. Model 47 is not quite as good at faces and bodies as 48 but is better than 49. The higher frequencies of faces and bodies seems to be overshadowed by interesting background patterns. Even if model 50 is no better than model 49 we still seem to have a model that focuses prefereably on the things that interest us. With multi-scale and multi-level and perhaps dynamics we can perhaps increase the focus on human-interesting objects more intensely. README

-->

<a name = "Conclusion"></a>

### Conclusion

<!-- TODO 

Could do an RGB capture instead of HSV. After all, this is how HB do it. Have to balance resolution, valency and type and bucketing to maximise contrast

Expect that even with multi-scale and edge detection we will probably still need a temporal higher level slice topology search to make the wotbot behaviour resemble a human. Possibly we will also need audio. The reason is that the features that are interesting to us - moving lips, for example - are often buried on shorter paths along with unrelated events deep in the model.

The things that are interesting to humans and other animals are those that natural selection has chosen - other agents, ripe fruit, water, heights, insects, teeth, eyes, etc. The lowest levels are those which depend most on hardwiring eg edge detection for other animals, and stereoscopic vision for distance calculation. At higher levels emotions seem to control the goal choices as agents navigate through the active slice topologies. For wotbot to be interesting to users it must be interested in the same things users are, so let biology be our guide while remembering that the larger context is the inanimate such as is seen in autistic interest in backgrounds and arrangements rather than agents.

This behaviour is excellent in the sense that likelihood is a good indicator for future potential, but shows that we need less cluttered frames - the model is quite poor for new scenes. Wotbot needs to be able to be able to identify the different objects independently at smaller scales, while having the overall scene at larger scales too. Should we extend the substrate to allow one full scale frame plus a dynamic variable scale frame? Or should that be arranged in levels, i.e. the higher level has temporal and two underlyings - the full scale scene and the scanned variable scale frame.

Note on how the contour maps enables us to tell where the model is most concentrated for a given scene. Then we can know if our modelling mode and parameters are tending to produce models that are interested in areas that are also interesting to humans, i.e. socially significant areas such as faces.

-->
