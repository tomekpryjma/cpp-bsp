#include <stdio.h>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/**
TODO: change to use tris instead of creating quads.
I am running into a problem where I need to determine the edge boundary of
a face quad to decide where the splitplane should add a new vertex (and therefore
    an additional edge in the face).
    Trying to do it this way seems overly complicated. Doing it the easier way and
    just joining all of the face vertices will cause triangle-esque edge connections anyway...
The trade off of using tris means that there might potentially be more coplanar faces to deal with
when deciding the infront/inback but that seems like a simpler problem that won't make things
super frustrating to deal with in the future.
 */
typedef struct _FaceTriangle {
    aiVector3f vertices[3];

    struct _SplitPlane* split_plane;
    bool is_used;
} FaceTriangle;

typedef struct _SplitPlane {
    aiVector3f normal;
    struct _FaceTriangle* face;
    bool is_used;
} SplitPlane;

// IDEA: put meshes and all of their verts in some sort of global array, i.e. [mesh1: [v1,v2,v3], mesh2: [v1,v2] ... ]
// this way i could pass around vert ids instead of 3d vecs.
typedef struct _BSPNode {
    SplitPlane* split_plane;
    struct _BSPNode* in_back;
    struct _BSPNode* in_front;
    bool is_leaf;
    std::vector<aiVector3f> geom_vertices;
} BSPNode;

bool bsp_init(std::vector<SplitPlane>& split_planes, std::vector<FaceTriangle>& faces) {
    BSPNode root;
    root.split_plane = &split_planes.at(0);
    split_planes.at(0).is_used = true;
    root.in_back = NULL;
    root.in_front = NULL;
    root.is_leaf = false;

    int num_split_planes = split_planes.size();
    int num_used_split_planes = 1;
    int max_iters = 500;
    int iter = 0;

    // Make sure to use next unused split plane.
    // Mark used planes.
    // while loop should ensure that it keeps running if not all split planes have been
    // processed yet.
    // split planes need to add verts/geom from faces (and therefore planes) that haven't been used yet.

    return 0;
}

void bsp(BSPNode current_node) {
    // get curr node split plane normal
    // loop through all face quads
    //  get edges of face
    //  see whether whole edge lies on one side or the other
    //  if one endpoint is one one side of splitplane and second endpoint
    //   on the other, then split the edge and add a vertex (might need to be duped)
};

int main(int argc, char** argv) {
    std::string test_data_dirpath = APP_ROOT;
        test_data_dirpath.append("/test-data");
    std::string level_filepath = test_data_dirpath + "/simple-plane-tilted.glb";

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(level_filepath, aiProcess_JoinIdenticalVertices);

    std::vector<SplitPlane> split_planes;
    std::vector<FaceTriangle> faces;

    int num_meshes = scene->mNumMeshes;

    for (int mi = 0; mi < num_meshes; mi++) {
        const aiMesh* mesh = scene->mMeshes[mi];
        int faces_num = mesh->mNumFaces;
        int tris_processed = 0;

        for (int fi = 0; fi < faces_num; fi++) {
            tris_processed++;
            const struct aiFace mesh_face = mesh->mFaces[fi];
            int num_verts_per_tri = 3;

            if (mesh_face.mNumIndices > num_verts_per_tri) {
                printf(
                    "Mesh[%d] -> Face[%d] has more than 3 verts...might cause trouble.",
                    mi, fi
                );
                exit(3);
            }

            FaceTriangle face;
            face.split_plane = NULL;

            for (int fvi = 0; fvi < num_verts_per_tri; fvi++) {
                int mesh_vertex_id = mesh_face.mIndices[fvi];
                face.vertices[fvi] = mesh->mVertices[mesh_vertex_id];
            }
            faces.push_back(face);
        }
    }

    // Generate splitplanes
    int num_faces = faces.size();
    for (int face_idx = 0; face_idx < num_faces; face_idx++) {
        FaceTriangle face = faces[face_idx];
        FaceTriangle* ptr_face = &faces[face_idx];
        // get normal of face (cross product)
        aiVector3f v1 = face.vertices[0];
        aiVector3f v2 = face.vertices[1];
        aiVector3f v3 = face.vertices[2];
        aiVector3f a(
            v2.x - v1.x, v2.y - v1.y, v2.z - v1.z
        );
        aiVector3f b(
            v3.x - v1.x, v3.y - v1.y, v3.z - v1.z
        );
        aiVector3f face_normal(
            a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x
        );
        face_normal.Normalize();
        SplitPlane sp;
        sp.normal = face_normal;
        sp.is_used = false;
        sp.face = ptr_face;
        split_planes.push_back(sp);
        ptr_face->split_plane = &split_planes.back();
    }

    int num_split_planes = split_planes.size();
    int bsp_res = bsp_init(split_planes, faces);
}