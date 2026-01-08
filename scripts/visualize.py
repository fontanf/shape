import argparse
import json
import plotly.graph_objects as go
import plotly.express as px
import plotly.subplots
import numpy as np
import math
import plotly.io as pio
pio.renderers.default = "browser"

def element_path(path_x, path_y, element, is_hole=False):
    # How to draw a filled circle segment?
    # https://community.plotly.com/t/how-to-draw-a-filled-circle-segment/59583
    # https://stackoverflow.com/questions/70965145/can-plotly-for-python-plot-a-polygon-with-one-or-multiple-holes-in-it
    t = element["type"]
    xs = element["start"]["x"]
    ys = element["start"]["y"]
    xe = element["end"]["x"]
    ye = element["end"]["y"]
    if t == "CircularArc":
        xc = element["center"]["x"]
        yc = element["center"]["y"]
        orientation = element["orientation"]
        rc = math.sqrt((xc - xs)**2 + (yc - ys)**2)

    if is_hole:
        xs, ys, xe, ye = xe, ye, xs, ys
        if t == "CircularArc":
            if orientation in ["Anticlockwise", "anticlockwise", "A", "a"]:
                orientation = "Clockwise"
            if orientation in ["Clockwise", "clockwise", "C", "c"]:
                orientation = "Anticlockwise"

    if len(path_x) == 0 or path_x[-1] is None:
        path_x.append(xs)
        path_y.append(ys)

    if t == "LineSegment":
        path_x.append(xe)
        path_y.append(ye)
    elif t == "CircularArc":
        start_cos = (xs - xc) / rc
        start_sin = (ys - yc) / rc
        start_angle = math.atan2(start_sin, start_cos)
        end_cos = (xe - xc) / rc
        end_sin = (ye - yc) / rc
        end_angle = math.atan2(end_sin, end_cos)
        if orientation in ["Full", "full", "F", "f"]:
            end_angle += 2 * math.pi
        if (orientation in ["Anticlockwise", "anticlockwise", "A", "a"]
                and end_angle <= start_angle):
            end_angle += 2 * math.pi
        if (orientation in ["Clockwise", "clockwise", "C", "c"]
                and end_angle >= start_angle):
            end_angle -= 2 * math.pi

        t = np.linspace(start_angle, end_angle, 1024)
        x = xc + rc * np.cos(t)
        y = yc + rc * np.sin(t)
        for xa, ya in zip(x[1:], y[1:]):
            path_x.append(xa)
            path_y.append(ya)


def shape_path(path_x, path_y, shape, is_hole=False):
    for element in (shape["elements"] if not is_hole else reversed(shape["elements"])):
        element_path(path_x, path_y, element, is_hole)
    path_x.append(None)
    path_y.append(None)


parser = argparse.ArgumentParser(description='')
parser.add_argument('path', help='path to JSON file')
args = parser.parse_args()

colors = px.colors.qualitative.Plotly
fig = go.Figure()

with open(args.path, 'r') as f:
    j = json.load(f)

    # Plot shapes with holes.
    if "shapes_with_holes" in j:
        for shape_pos, shape in enumerate(j["shapes_with_holes"]):
            shape_x = []
            shape_y = []

            shape_path(shape_x, shape_y, shape)
            for hole in (shape["holes"]
                         if "holes" in shape else []):
                shape_x.append(None)
                shape_y.append(None)
                shape_path(shape_x, shape_y, hole, True)

            fig.add_trace(go.Scatter(
                x=shape_x,
                y=shape_y,
                name=f"Shape with holes {shape_pos}",
                # legendgroup=filepath,
                    showlegend=True,
                    fillcolor=colors[shape_pos % len(colors)],
                    opacity=0.2,
                    fill="toself",
                    marker=dict(
                        color='black',
                        size=1)))

    # Plot shapes.
    if "shapes" in j:
        for shape_pos, shape in enumerate(j["shapes"]):
            shape_x = []
            shape_y = []

            shape_path(shape_x, shape_y, shape)

            fig.add_trace(go.Scatter(
                x=shape_x,
                y=shape_y,
                name=f"Shape {shape_pos}",
                # legendgroup=filepath,
                    showlegend=True,
                    fillcolor=colors[shape_pos % len(colors)],
                    opacity=0.2,
                    fill="toself",
                    marker=dict(
                        color='black',
                        size=1)))

    # Plot elements.
    if "elements" in j:
        element_x = []
        element_y = []
        for element_pos, element in enumerate(j["elements"]):
            element_path(element_x, element_y, element)
            element_x.append(None)
            element_y.append(None)

            fig.add_trace(go.Scatter(
                x=element_x,
                y=element_y,
                name=f"Element {element_pos}",
                # legendgroup=filepath,
                showlegend=True,
                opacity=0.2,
                marker=dict(
                    color='black',
                    size=1)))


# Plot.
fig.update_layout(
        autosize=True)
fig.update_yaxes(
        scaleanchor="x",
        scaleratio=1)
fig.show()
