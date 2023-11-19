import json
import os
import random
import math
color = [[0.5, 0.3, 0.9], [0.9, 0.3, 0.5], [0.5, 0.9, 0.3], [0.3, 0.9, 0.5], [0.3, 0.5, 0.9]]
class Ball:
    def __init__(self, id, position, velocity):
        self.id = id
        self.position = position
        self.velocity = velocity
        self.color = color[self.id-1]

def update_ball(ball, time_step, gravity, ground_level, ball_radius, horizontal_speed):
    # Apply gravity
    ball.velocity[1] -= gravity * time_step

    # Update position
    ball.position[0] += ball.velocity[0] * time_step
    ball.position[1] += ball.velocity[1] * time_step
    ball.position[2] += ball.velocity[2] * time_step

    # Check for collision with the ground
    if ball.position[1] < ground_level + ball_radius:
        ball.position[1] = ground_level + ball_radius
        ball.velocity[1] = -ball.velocity[1] * 0.8  # Reverse vertical velocity
        if ball.velocity[0] == 0:
            # Randomize horizontal velocity after bounce
            angle = random.uniform(0, 2 * math.pi)
            ball.velocity[0] = horizontal_speed * math.cos(angle)
            ball.velocity[2] = horizontal_speed * math.sin(angle)

def generate_frame_json(balls, frame_number, camera_radius=5):
    # Calculate the camera position
    angle = (frame_number / total_frames) * 2 * math.pi  # Full circle over the total frames
    camera_x = camera_radius * math.cos(angle)
    camera_z = camera_radius * math.sin(angle) + 3 
    frame_data = {
        "nbounces": 8,
        "rendermode": "phong",
        "camera": {
            "type": "pinhole",
            "width": 1200,
            "height": 800,
            "position": [camera_x, 1, camera_z],  # Updated camera position
            "lookAt": [0.0, -0.5, 3.0],          # Constant lookAt position
            "upVector": [0.0, 1.0, 0.0],
            "fov": 30.0,
            "exposure": 0.1
        },
        "scene": {
            "backgroundcolor": [0.25, 0.25, 0.25],
            "lightsources": [
                {
                    "type": "pointlight",
                    "position": [0, 1.2, 3],
                    "intensity": [0.8, 0.8, 0.8]
                }
            ],
            "shapes": [
                {
                    "type": "triangle",
                    "v0": [-3, -0.5, 6],
                    "v1": [3, -0.5, 6],
                    "v2": [3, -0.5, 0],
                    "material": {
                        "ks": 0.1,
                        "kd": 0.9,
                        "specularexponent": 20,
                        "diffusecolor": [0.8, 0.8, 0.8],
                        "specularcolor": [1.0, 1.0, 1.0],
                        "isreflective": False,
                        "reflectivity": 1.0,
                        "isrefractive": False,
                        "refractiveindex": 1.0
                    }
                },
                {
                    "type": "triangle",
                    "v0": [-3, -0.5, 0],
                    "v1": [-3, -0.5, 6],
                    "v2": [3, -0.5, 0],
                    "material": {
                        "ks": 0.1,
                        "kd": 0.9,
                        "specularexponent": 20,
                        "diffusecolor": [0.8, 0.8, 0.8],
                        "specularcolor": [1.0, 1.0, 1.0],
                        "isreflective": False,
                        "reflectivity": 1.0,
                        "isrefractive": False,
                        "refractiveindex": 1.0
                    }
                }] + [
                {
                    "type": "sphere",
                    "center": ball.position,
                    "radius": ball_radius,
                    "material": {
                        "ks": 0.1,
                        "kd": 0.9,
                        "specularexponent": 20,
                        "diffusecolor": ball.color,
                        "specularcolor": [0.0, 0.0, 0.0],
                        "isreflective": False,
                        "reflectivity": 1.0,
                        "isrefractive": False,
                        "refractiveindex": 1.0
                    }
                } for ball in balls
            ]
        }
    }
    return frame_data

def create_animation_json(total_frames, initial_height, gravity, ball_radius, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    balls = []
    for frame in range(total_frames):
        if frame % 48 == 0:  # Every 2 seconds (48 frames)
            balls.append(Ball(len(balls), [0, initial_height, 3], [0, 0, 0]))

        for ball in balls:
            update_ball(ball, 1 / 24, gravity, -0.5, ball_radius, 0.35)  # Sample values for bounce and horizontal speed

        frame_data = generate_frame_json(balls, frame)
        with open(os.path.join(output_dir, f"frame_{frame:04d}.json"), "w") as file:
            json.dump(frame_data, file, indent=4)

# Parameters
fps = 24
duration_sec = 10
total_frames = fps * duration_sec
initial_height = 1  # Height from which the balls start falling
gravity = 9.81       # Gravity constant
ball_radius = 0.05    # Radius of the balls
output_dir = "data/animation_frames"  # Directory to save the JSON files

# Generate the animation JSON files
create_animation_json(total_frames, initial_height, gravity, ball_radius, output_dir)
