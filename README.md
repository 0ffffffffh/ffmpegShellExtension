ffmpeg Windows Explorer Shell Extension

ffmpegShell is my one of the "personal use" projects (because, i'm bored of typing command for ffmpeg everytime) 

So, it lets you to manipulate your media files from within the Windows Explorer shell menu.  You can define your own ffmpeg conversion commands as a preset structure. ffmpegShell compiles, detects media and operation type and appears as a menu command. Click the command menu and voila. The magic happens.

ffmpegShell is in Alpha stage. 


ffmpegShell Preset

Preset object is an internal structure type of the ffmpeg command informations.
You can define multiple preset in a single preset file.

Here is the preset structure


```
preset 
{
	[PRESET_PROPERTY] : [PRESET_PROPERTY_VALUE];
	 ....
	 ....
};
```

< character is comment out begin, and > character is the comment out end block

[PRESET_PROPERTY] can be following values

"name" (string) : it contains preset description. this value will be appear as the menu command text.

"command" (string) : ffmpeg conversion command. $INF{[ext1...extn]} and $OUTF constants represents the input and output filename for ffmpegShell Internal. 


"mediaType" (mediatype) : media file type. it can be one of the {audio, video} values.

"operationType" (optype) : ffmpeg command operation type. ffmpegShell will be using this value for seperation command category. it can be one of the 
{combination, extraction, conversion} values.

"sourceFormat" (string) : source media file extension.

"destinationFormat" (string) : output file format extension

"runOnFinish" (string) : command to execute after the process completes.

```
preset
{
	name : "Embed subtitle";
	command : "-i $INF[mp4] -vf subtitles=$INF[srt,sub] $OUTF";
	mediaType : video;
	operationType : combination;
	sourceFormat : "mp4";
};

```
