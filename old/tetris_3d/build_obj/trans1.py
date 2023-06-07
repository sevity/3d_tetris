from collections import defaultdict

def merge_vertices_and_optimize_lines(input_file_name, output_file_name):
    with open(input_file_name, 'r') as file:
        lines = file.readlines()

    vertices = []
    faces = []
    vertices_map = defaultdict(list)
    for line in lines:
        if line.startswith('v'):
            coords = tuple(map(float, line.split()[1:]))
            vertices.append(coords)
            vertices_map[coords].append(len(vertices))
        elif line.startswith('f'):
            indices = list(map(int, line.split()[1:]))
            faces.append(indices)

    # Merge vertices and update faces
    new_vertices_map = {coords: min(indices) for coords, indices in vertices_map.items()}
    new_vertices = [coords for coords in new_vertices_map]
    new_vertices_indices = {coords: i+1 for i, coords in enumerate(new_vertices)}
    new_faces = [[new_vertices_indices[vertices[i-1]] for i in indices] for indices in faces]

    edges = defaultdict(int)
    for indices in new_faces:
        for i in range(len(indices)):
            edge = tuple(sorted([indices[i], indices[(i+1)%len(indices)]]))
            edges[edge] += 1

    with open(output_file_name, 'w') as file:
        for coords in new_vertices:
            file.write(f"v {' '.join(map(str, coords))}\n")
        for indices in new_faces:
            for i in range(len(indices)):
                edge = tuple(sorted([indices[i], indices[(i+1)%len(indices)]]))
                if edges[edge] > 1:
                    edges[edge] -= 1
                else:
                    file.write(f"l {edge[0]} {edge[1]}\n")

merge_vertices_and_optimize_lines("input.obj", "output.obj")

