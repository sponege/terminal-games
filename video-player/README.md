# Terminal Video Player

[![asciicast](https://asciinema.org/a/kJ88C2yuonxKEBjNDZgJ0205P.svg)](https://asciinema.org/a/kJ88C2yuonxKEBjNDZgJ0205P)

To install:

```
sudo apt install libpng-dev
sudo apt install ffmpeg
cd ..
make video-player
```

Then use `ffmpeg` to convert your video to several different frames. Make sure the `frames/` folder is in `build/`

```
ffmpeg -i astley.mp4 -vf fps=60 frames/frame%d.png
```

Then you can play the video as follows:

```
cd build
./video
```

Default number of frames used is 100. You can change this in `config.h`

Shout-out to [LennyPhoenix](https://github.com/LennyPhoenix) for reading and fixing my messy code!

He made my terminal video player actually work! :O
