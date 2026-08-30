import subprocess
import os

BASE = '/workspace/promovideo'
OUT = os.path.join(BASE, 'out')
os.makedirs(OUT, exist_ok=True)

clips = [
    ('clip01_title',        0),
    ('clip02_day_fishing', 15),
    ('clip03_dusk_catch',  30),
    ('clip04_night_storm', 45),
    ('clip05_shop',        60),
    ('clip06_sunset_ending', 75),
]

force = ("FontName=Noto Sans CJK SC,FontSize=44,"
         "PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,"
         "BorderStyle=1,Outline=2,Shadow=0,Alignment=2,MarginV=42")

for name, start in clips:
    v_in = os.path.join(BASE, name + '.mp4')
    srt = os.path.join(BASE, 'subs', name + '.srt')
    v_out = os.path.join(OUT, name + '.mp4')

    fc = ("[0:v]delogo=x=1150:y=668:w=120:h=45,"
          f"subtitles={srt}:force_style='{force}'[v]")

    cmd = [
        'ffmpeg', '-y', '-hide_banner', '-loglevel', 'error',
        '-i', v_in,
        '-ss', str(start), '-t', '15', '-i', os.path.join(BASE, 'music.wav'),
        '-filter_complex', fc,
        '-map', '[v]', '-map', '1:a',
        '-c:v', 'libx264', '-preset', 'medium', '-crf', '18', '-pix_fmt', 'yuv420p',
        '-c:a', 'aac', '-b:a', '160k',
        '-shortest', v_out,
    ]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print('FAIL', name)
        print(r.stderr[-2000:])
    else:
        print('OK', name, '->', v_out)