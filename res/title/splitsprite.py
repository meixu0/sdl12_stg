import cv2
import numpy as np

# 1. 读取图片
image_path = 'title01.png'
img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)

if img is None:
    print("无法读取图片，请检查文件名和路径是否正确！")
    exit()

# 2. 生成检测遮罩 (Mask)
# 判断是否有 3 个维度，且第 3 个维度（通道数，索引为2）是否为 4 (包含 Alpha 通道)
if len(img.shape) == 3 and img.shape[2] == 4:
    print("检测到透明通道，使用 Alpha 通道提取...")
    # 提取第 4 个通道 (索引为 3)
    alpha_channel = img[:, :, 3]
    _, mask = cv2.threshold(alpha_channel, 10, 255, cv2.THRESH_BINARY)
else:
    print("未检测到透明通道，自动切换为暗色背景提取模式...")
    # 如果只有 RGB，转为灰度图
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # 设定一个亮度阈值（比如 50），低于 50 的深灰色背景变黑，文字变白
    # 如果框选不准，可以稍微调高或调低这个数字 '50'
    _, mask = cv2.threshold(gray, 50, 255, cv2.THRESH_BINARY)

# 3. 寻找轮廓 (Contours)
contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# 4. 遍历轮廓并获取 Bounding Box 坐标
coordinates = []
for cnt in contours:
    x, y, w, h = cv2.boundingRect(cnt)
    # 过滤掉太小的噪点（宽高小于5像素的忽略）
    if w > 5 and h > 5:
        coordinates.append({"x": x, "y": y, "width": w, "height": h})
        # 在原图上画出绿框用于预览
        cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0, 255), 1)

# 按 Y 坐标排序，再按 X 坐标排序，让输出的坐标顺序更符合人类阅读习惯（从上到下，从左到右）
coordinates.sort(key=lambda c: (c['y'] // 10, c['x'])) 

# 打印结果
for i, coord in enumerate(coordinates):
    print(f"Sprite {i:02d}: X={coord['x']}, Y={coord['y']}, W={coord['width']}, H={coord['height']}")

# 保存带框的预览图
cv2.imwrite('preview_with_boxes.png', img)
print(f"提取完成！共找到 {len(coordinates)} 个精灵图块。请查看目录下的 preview_with_boxes.png 检查框选是否准确。")
