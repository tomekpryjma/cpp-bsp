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
typedef struct _FaceQuad {
    aiVector3f vertices[4];

    struct _SplitPlane* split_plane;
    bool is_used;
} FaceQuad;

typedef struct _SplitPlane {
    aiVector3f normal;
    struct _FaceQuad* face_quad;
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

bool bsp_init(std::vector<SplitPlane>& split_planes, std::vector<FaceQuad>& face_quads) {
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
    std::vector<FaceQuad> face_quads;

    int num_meshes = scene->mNumMeshes;
    int tris_per_quad = 2;

    for (int mi = 0; mi < num_meshes; mi++) {
        const aiMesh* mesh = scene->mMeshes[mi];
        int faces_num = mesh->mNumFaces;
        int tris_processed = 0;
        std::vector<int> vert_ids_processed;
        vert_ids_processed.reserve(4);
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

            for (int fvi = 0; fvi < num_verts_per_tri; fvi++) {
                bool mesh_vertex_already_added = false;
                int mesh_vertex_id = mesh_face.mIndices[fvi];
                for (int exist_vert_lookup_idx = 0; exist_vert_lookup_idx < 4; exist_vert_lookup_idx++) {
                    if (exist_vert_lookup_idx < vert_ids_processed.size()) {
                        if (vert_ids_processed[exist_vert_lookup_idx] == mesh_vertex_id) {
                            mesh_vertex_already_added = true;
                            break;
                        }
                    }
                }
                if (!mesh_vertex_already_added) {
                    vert_ids_processed.push_back(mesh_vertex_id);
                } 
            }

            if (tris_processed == 2) { // quad face complete
                tris_processed = 0;
                FaceQuad face_quad;
                face_quad.split_plane = NULL;
                for (int vert_id_idx = 0; vert_id_idx < 4; vert_id_idx++) {
                    // get actual vec 3f
                    int mesh_vertex_id = vert_ids_processed[vert_id_idx];
                    face_quad.vertices[vert_id_idx] = mesh->mVertices[mesh_vertex_id];
                }
                face_quads.push_back(face_quad);
                vert_ids_processed.clear();
            }
        }
    }

    // Generate splitplanes
    int num_face_quads = face_quads.size();
    for (int face_quad_idx = 0; face_quad_idx < num_face_quads; face_quad_idx++) {
        FaceQuad face_quad = face_quads[face_quad_idx];
        FaceQuad* ptr_face_quad = &face_quads[face_quad_idx];
        // get normal of face (cross product)
        aiVector3f v1 = face_quad.vertices[0];
        aiVector3f v2 = face_quad.vertices[1];
        aiVector3f v3 = face_quad.vertices[2];
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
        sp.face_quad = ptr_face_quad;
        split_planes.push_back(sp);
        ptr_face_quad->split_plane = &split_planes.back();
    }

    int num_split_planes = split_planes.size();
    int bsp_res = bsp_init(split_planes, face_quads);
}