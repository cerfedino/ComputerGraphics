# openrender: render/result.png
# 	(xdg-open render/result.png | open render/result.png)
FLAGS = -O3 -fopenmp
# Framerate of the exported animation
FPS = 25

# Recipes for rendering the still frame
render/result.png: main.out render/
	./main.out render/result.ppm && convert "render/result.ppm" "render/result.png"
main.out: src/main.cpp src/renderer.h
	g++ $(FLAGS) src/main.cpp src/renderer.h -o main.out

# Removes all generated files
clean:
	rm -r  render/ *.out
# Recipe for rendering everything
all: render/result.png animation


# Recipes for rendering the animation
animation: render/result.gif
render/result.gif: render/result.mp4
	ffmpeg -y -framerate $(FPS) -pattern_type glob -i 'render/animation/*.ppm' -loop 0 -vf scale=800x600 render/result.gif
render/result.mp4: animation.out render/animation/
	rm -r render/animation/* &> /dev/null
	./animation.out $(FPS) && ffmpeg -y -framerate $(FPS) -pattern_type glob -i 'render/animation/*.ppm' -c:v libx264 -pix_fmt yuv420p render/result.mp4
animation.out: src/animation.cpp src/renderer.h
	g++ src/animation.cpp src/renderer.h $(FLAGS) -o animation.out

# Recipe for the renderer
src/renderer.h:
	g++ src/renderer.h $(FLAGS)

# Recipes for exported renders folders
render/animation/: render/
	mkdir -p render/animation
render/:
	mkdir -p render/