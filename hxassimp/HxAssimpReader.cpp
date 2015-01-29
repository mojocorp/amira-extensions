#include <hxassimp/api.h>

#include <hxcore/HxMessage.h>
#include <hxcore/HxWorkArea.h>

#include <hxsurface/HxSurface.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>

static void get_mesh(const aiScene* scene, const aiNode* node, HxSurface* surface, const aiMatrix4x4& accTransform) {
    aiMatrix4x4 norm_trafo = accTransform;
    norm_trafo.Inverse();
    norm_trafo.Transpose();

    const aiMatrix4x4 model_trafo = accTransform * node->mTransformation;

    for (unsigned int n = 0; n < node->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

        // Points and Lines are not handled, only surfaces.
        if ((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
            continue;

        assert(mesh->HasNormals());

        const int currPatchIdx = surface->patches.size();

        Surface::Patch* patch = new Surface::Patch();
        patch->innerRegion = 0;
        patch->outerRegion = mesh->mMaterialIndex + 1;
        patch->triangles.resize(mesh->mNumFaces);

        surface->patches.append(patch);

        const int currPointIdx = surface->points.size();
        const int currTriIdx = surface->triangles.size();

        surface->points.resize(currPointIdx + mesh->mNumVertices);
        surface->normals.resize(currPointIdx + mesh->mNumVertices);
        surface->triangles.resize(currTriIdx + mesh->mNumFaces);

        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {

            aiVector3D vert = mesh->mVertices[v];
            vert *= model_trafo;

            const aiVector3D norm = norm_trafo * mesh->mNormals[v];

            surface->points[currPointIdx + v].setValue(vert.x, vert.y, vert.z);
            surface->normals[currPointIdx + v].setValue(norm.x, norm.y, norm.z);
        }

        for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
            const aiFace& face = mesh->mFaces[t];
            Surface::Triangle& tri = surface->triangles[currTriIdx + t];
            tri.points[0] = currPointIdx + face.mIndices[0];
            tri.points[1] = currPointIdx + face.mIndices[1];
            tri.points[2] = currPointIdx + face.mIndices[2];
            tri.patch = currPatchIdx;

            patch->triangles[t] = currTriIdx + t;
        }
    }

    for (unsigned int n = 0; n < node->mNumChildren; ++n) {
        get_mesh(scene, node->mChildren[n], surface, model_trafo);
    }
}

void get_materials(const aiScene* scene, HxSurface* surface) {
    surface->addMaterial("Inside");
    if (scene->mNumMaterials == 0) {
        surface->addMaterial("Exterior");
    }

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* material = scene->mMaterials[i];

        aiString name;
        material->Get(AI_MATKEY_NAME, name);

        int matidx = surface->addMaterial(name.C_Str());

        HxParamBundle* bundle = surface->parameters.materials()->bundle(matidx);

        aiColor4D color;
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color)) {
            bundle->insert(new HxParameter("Color", 3, &color[0]));
            bundle->insert(new HxParameter("diffuse", 3, &color[0]));
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &color)) {
            bundle->insert(new HxParameter("ambient", 3, &color[0]));
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &color)) {
            bundle->insert(new HxParameter("specular", 3, &color[0]));
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &color)) {
            bundle->insert(new HxParameter("emissive", 3, &color[0]));
        }
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_SHININESS, &color)) {
            bundle->insert(new HxParameter("shininess", 3, &color[0]));
        }
    }
}

class ProgressHandler : public Assimp::ProgressHandler {
  public:
    ProgressHandler(McProgressInterface* progress) : progress(progress) {
        progress->startWorking("Loading model...");
    }

    ~ProgressHandler() {
        progress->stopWorking();
    }

    virtual bool Update(float percentage = -1.f) {
        progress->setProgressValue(percentage);
        return !progress->wasInterrupted();
    }

  private:
    McProgressInterface* progress;
};

HXASSIMP_API
int hxAssimpReader(const char* filename) {
    McHandle<HxSurface> surface = new HxSurface();
    surface->normalBinding = Surface::PER_VERTEX;

#ifdef _DEBUG
    aiLogStream stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
    aiAttachLogStream(&stream);
#endif

    Assimp::Importer importer;
    importer.SetProgressHandler(new ProgressHandler(theWorkArea));

    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals);

    if (!scene) {
        theMsg->ioError(filename, importer.GetErrorString());
        return 0;
    }

    // Create materials
    get_materials(scene, surface);

    // Create geometry
    aiMatrix4x4 trafo;
    aiIdentityMatrix4(&trafo);

    get_mesh(scene, scene->mRootNode, surface, trafo);

#ifdef _DEBUG
    aiDetachAllLogStreams();
#endif

    HxData::registerData(surface, filename);

    return 1;
}
