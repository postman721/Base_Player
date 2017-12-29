# Base_Player
Base Player is a C++ and QT5 music player

Base Player will deliver all the features of the Albix Player, while adding a few more. The two projects in question will continue to grow separately.

<Base Player v.1 Copyright (c) 2017 JJ Posti <techtimejourney.net>
<Base Player v.1> comes with ABSOLUTELY NO WARRANTY;
for details see: http://www.gnu.org/copyleft/gpl.html.
This is free software, and you are welcome to redistribute it under
GPL Version 2, June 1991″
_____________________________________






![base](https://user-images.githubusercontent.com/29865797/34427319-de56b5dc-ec49-11e7-81ed-85e7a023b7ce.png)

Current features:

    Add or remove files. When removing song make sure that you remove the topmost song last. When the topmost song is remove the remove button will go disabled. Remove button activates again when a new list of songs arrives.
    
    Play, Stop,Pause playback.
    
    Accepts songs from different folders as items for the playback.
    
    Duration bar that moves according to the song position. Can be used for navigation as well.
    
    Volume adjuster that uses pulseaudio and pulseaudio-utils.
    
    All buttons and elements have tooltips.
    
    Done with pure QT5.
    
    Styled with CSS.

TODO:

Add a full playlist support with a continuous playback.
____________________

Building:


Generally, you should be good to go when you install something like this:


sudo yum install qt5-qtmultimedia-devel qt5-qtbase-devel .


Also, be sure to install Qtcreator (qtcreator package) because you need to  build the project with it. 

____________________________

Original post is at:
