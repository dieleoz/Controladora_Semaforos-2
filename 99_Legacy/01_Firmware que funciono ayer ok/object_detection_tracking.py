import numpy as np
import datetime
import cv2
from ultralytics import YOLO

from helper import create_video_writer

conf_threshold = 0.5

# Initialize the video capture and the video writer objects
video_cap = cv2.VideoCapture("1.mp4")
writer = create_video_writer(video_cap, "output.mp4")
# Initialize the YOLOv8 model using the default weights
model = YOLO("yolov8s.pt")