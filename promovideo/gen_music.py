import math
import wave
from array import array

SR = 44100
SEG = 15.0
DUR = 90.0
N = int(SR * DUR)

# 和弦（Hz，每段 15 秒一个，共 6 段）
chords = [
    [130.81, 164.81, 196.00, 246.94],   # 1 Cmaj7  黎明/片头
    [110.00, 130.81, 164.81, 196.00],   # 2 Am7    白天
    [87.31, 110.00, 130.81, 164.81],    # 3 Fmaj7  黄昏中鱼
    [73.42, 87.31, 110.00, 130.81],     # 4 Dm7    暴风雨
    [98.00, 123.47, 146.83, 185.00],    # 5 Gmaj7  商店
    [130.81, 164.81, 196.00, 293.66],   # 6 Cadd9  日落收尾
]

# 琶音音高（在更高的八度，每段一个上行/下行模式）
arp_patterns = []
for c in chords:
    up = [c[0] * 4.0, c[1] * 4.0, c[2] * 4.0, c[3] * 4.0, c[0] * 8.0]
    arp_patterns.append(up[:-1][::-1] + up)  # 下行接上行

L = array('f', [0.0]) * N
R = array('f', [0.0]) * N
pi2 = 2.0 * math.pi

def smoothstep(x):
    if x <= 0.0:
        return 0.0
    if x >= 1.0:
        return 1.0
    return x * x * (3.0 - 2.0 * x)

# 主循环
for i in range(N):
    t = i / SR
    seg = int(t // SEG)
    if seg > 5:
        seg = 5

    # 段落内时间
    ts = t - seg * SEG

    # ---- Pad：跨段交叉淡化 ----
    # 当前和弦 pad
    cur_val = 0.0
    for f in chords[seg]:
        cur_val += math.sin(pi2 * f * t)
    cur_val /= len(chords[seg])

    # 上一和弦 pad（段边界交叉淡化用）
    prev_val = 0.0
    if seg > 0:
        for f in chords[seg - 1]:
            prev_val += math.sin(pi2 * f * t)
        prev_val /= len(chords[seg - 1])
    else:
        prev_val = cur_val

    # 段首 1.2 秒做交叉淡化
    blend = 1.0
    if ts < 1.2:
        blend = smoothstep(ts / 1.2)
    pad = prev_val * (1.0 - blend) + cur_val * blend

    # 呼吸感（LFO 颤音）
    trem = 0.9 + 0.1 * math.sin(pi2 * 0.15 * t)
    pad *= trem * 0.115

    # ---- 低音 ----
    root = chords[seg][0] * 0.5
    bass = math.sin(pi2 * root * t) * 0.13

    # ---- 旋律（琶音，轻柔拨弦质感）----
    mel = 0.0
    slot = int(ts // 1.5)
    if slot < len(arp_patterns[seg]):
        freq = arp_patterns[seg][slot]
        # 每个音的局部起始时间
        ts_note = ts - slot * 1.5
        # 快速起音 + 指数衰减
        env = math.exp(-2.2 * ts_note)
        if ts_note < 0.01:
            env = ts_note / 0.01
        # 加一个八度泛音让拨弦更亮
        mel = (math.sin(pi2 * freq * ts_note) +
               0.25 * math.sin(pi2 * freq * 2.0 * ts_note)) * env * 0.17
    else:
        mel = 0.0

    v = pad + bass + mel

    # 简单立体声：旋律向两侧轻微摆动
    pan = 0.5 + 0.5 * math.sin(pi2 * 0.05 * t * 0.5) if mel != 0.0 else 0.5
    gl = math.sqrt(1.0 - pan)
    gr = math.sqrt(pan)

    L[i] = v * (0.7 + 0.3 * gl)
    R[i] = v * (0.7 + 0.3 * gr)

# 归一化
peak = 0.0
for i in range(N):
    a = abs(L[i])
    if a > peak:
        peak = a
    a = abs(R[i])
    if a > peak:
        peak = a

scale = 0.85 / peak if peak > 0 else 1.0

frames = bytearray()
for i in range(N):
    sl = int(max(-1.0, min(1.0, L[i] * scale)) * 32767)
    sr_ = int(max(-1.0, min(1.0, R[i] * scale)) * 32767)
    frames += sl.to_bytes(2, 'little', signed=True)
    frames += sr_.to_bytes(2, 'little', signed=True)

with wave.open('/workspace/promovideo/music.wav', 'wb') as w:
    w.setnchannels(2)
    w.setsampwidth(2)
    w.setframerate(SR)
    w.writeframes(bytes(frames))

print('done', N, 'samples peak', round(peak, 3))