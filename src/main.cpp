#include <stdio.h>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef struct _FaceQuad {
    aiVector3f vertices[4];
    bool is_used;
} FaceQuad;

typedef struct _SplitPlane {
    aiVector3f normal;
    aiVector3f direction;
} SplitPlane;

int main(int argc, char** argv) {
    std::string test_data_dirpath = APP_ROOT;
        test_data_dirpath.append("/test-data");
    std::string level_filepath = test_data_dirpath + "/simple-plane.glb";

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(level_filepath, aiProcess_JoinIdenticalVertices);

    std::vector<FaceQuad> face_quads;
    std::vector<SplitPlane> split_planes;

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
}