# audiojoe - An Automated Porn Movie Jerk-Off Instructor

This is a program that spawned from the development of my other program, Porn Movie Orgasm Detector.

Like Porn Movie Orgasm Detector, this program analyses the sound from a movie and finds the points of highest sound intensity. The difference is that this program then designs a small schedule of build up, edge, and/or orgasm for the viewer to follow, based on the analyzed audio. The program generates the instructions/encouragements and writes them as subtitles to be played along the movie. These subtitles are saved as separate .srt files and can thus be reused and shared with others, if so desired.

The chance to cum is configured by the user when running the shell script.

Like Porn Movie Orgasm Detector, this program requires no external libraries beyond the OS-specific ones. It requires GCC, FFMPEG and VLC to be installed on your computer. It can be compiled and run on Windows or any Unix-like OS.

Now, there's an element of chance of exactly how this schedule is generated. There's a countdown and then sometimes you are allowed to cum, sometimes you are told to hold it until the next climax in the movie, sometimes you are denied on the last possible moment right before orgasm.

# What it does
When running it, it will take some time (seconds to minutes depending on your computer and the size of the video) before anything happens since FFmpeg has to convert the movie audio to WAV before sending it to the program. VLC will then start. The program generates the instructions/encouragements and writes them as subtitles to be played along the movie. Each time you run the program, the subtitles are going to be different so maybe you can try again tomorrow if denied today.

# How to use it on Windows
(1) Extract all the files from the .zip archive to a folder.

(2A). Find a movie to analyze and drag-and-drop it onto the audiojoe.bat file.

OR:

(2B). Open a command line prompt (Windows-button -> Run -> Cmd.exe) and navigate to the correct folder.
Suppose the movie you want to analyze is called Movie1.wmv. In that case, enter

    audiojoe.bat Movie1.wmv
    
and press Enter. 

(3). Finally, enter the chance to cum as a whole number between 0 and 100 or just press Enter if 50 % is okay.

# How to use it on Unix
(1). Extract all the files from the .zip archive to a folder.

(2A) Find a movie to analyze and drag-and-drop it onto the audiojoe.sh file.

OR 

(2B) Open a terminal window and navigate to the correct folder.
Suppose the movie you want to analyze is called Movie1.wmv.
In that case, enter

    ./audiojoe.sh Movie1.wmv

and press Enter. 

(3). Finally, enter the chance to cum as a whole number between 0 and 100 or just press Enter if 50 % is okay.

# Compiling from source on Windows
Compiling and archiving can be done in one command using the create-release-windows.sh script. Observe that this requires a Bash shell to run, which is included in Cygwin or Msysgit, for example.

If you just want to compile the program:

In the same directory as the .c file simply run:

    gcc audiojoe.c -o audiojoe.exe
    
# Compiling from source on *nix    
Compiling and archiving can be done in one command using the create-release-unix.sh script.
If you just want to compile the program:

In the same directory as the .c file simply run:

    gcc audiojoe.c -lrt -o audiojoe


