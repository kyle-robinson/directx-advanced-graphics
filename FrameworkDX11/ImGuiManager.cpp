#include "ImGuiManager.h"
#include"DrawableGameObject.h"
#include"LightController.h"
#include"ShaderController.h"
#include"BillboradObject.h"
#include"Terrain.h"
#include"TerrainVoxel.h"
#include"RasterizerController.h"
#include"AnimatedModel.h"



ImGuiManager::ImGuiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	SetBlackGoldStyle();
}

ImGuiManager::~ImGuiManager()
{
	ImGui::DestroyContext();
}

void ImGuiManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);
}


void ImGuiManager::BeginRender()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::EndRender()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//cam controll menu
static const char* current_item = NULL;
static string name;
static bool Load = false;
void ImGuiManager::DrawCamMenu(CameraController* Cams)
{
    ImGui::ShowMetricsWindow();
    if (!Load) {
        name =Cams->GetCurentCam()->GetCamName();
        current_item = name.c_str();
        Load = true;
    }

    if (ImGui::Begin("Cam Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::CollapsingHeader("Camera Controls"))
        {
            ImGui::Text("W                 Forward");
            ImGui::Text("A                 Left");
            ImGui::Text("S                 Backward");
            ImGui::Text("D                 Right");
            ImGui::Text("Leftmouse + move                 Look");
            ImGui::Separator();

        }
        if (ImGui::CollapsingHeader("Cam Select"))
        {

            if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < Cams->GetCamList().size(); n++)
                {
                    bool is_selected = (current_item == Cams->GetCamList()[n]->GetCamName().c_str()); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(Cams->GetCamList()[n]->GetCamName().c_str(), is_selected)) {

                        name = Cams->GetCamList()[n]->GetCamName();
                        Cams->SetCam(n);
                        current_item = name.c_str();
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)

                    }

                }
                ImGui::EndCombo();
            }
        }
    }
    ImGui::End();
}

//post processing and shader controll menus
static const char* current_Shader = NULL;
static string Shadername;
static bool LoadShader = false;
static string current_RasterState = "";
void ImGuiManager::ShaderMenu(ShaderController* Shader, PostProcessingCB* postSettings, RasterizerController* RasterState, bool& rtt)
{
    if (!LoadShader) {
        Shadername = Shader->GetShaderData().m_sName;
        current_Shader = Shadername.c_str();



        LoadShader = true;
    }

    if (ImGui::Begin("Shader Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {

        if (ImGui::CollapsingHeader("Shader Select"))
        {

            if (ImGui::BeginCombo("##combo", current_Shader)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < Shader->GetShaderList().size(); n++)
                {
                    bool is_selected = (current_Shader == Shader->GetShaderList()[n].m_sName.c_str()); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(Shader->GetShaderList()[n].m_sName.c_str(), is_selected)) {

                        Shadername = Shader->GetShaderList()[n].m_sName;
                        Shader->SetShaderData(n);
                        current_Shader = Shadername.c_str();
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)

                    }

                }
                ImGui::EndCombo();
            }



        }
        if (ImGui::CollapsingHeader("Post Processing"))
        {
            PostProcessingCB* currentPPCB= postSettings;

            bool useColour = currentPPCB->UseColour;
            ImGui::Checkbox("Colour Change", &useColour);
            currentPPCB->UseColour = useColour;

            float Colour[] = { currentPPCB->Color.x , currentPPCB->Color.y, currentPPCB->Color.z, currentPPCB->Color.w};
            ImGui::ColorPicker4("Colour", Colour, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
            currentPPCB->Color = { Colour[0],Colour[1],Colour[2],Colour[3] };

            bool useBloom = currentPPCB->UseBloom;
            ImGui::Checkbox("Bloom", &useBloom);
            currentPPCB->UseBloom = useBloom;

            bool useDOF = currentPPCB->UseDepthOfF;
            ImGui::Checkbox("DepthOfField", &useDOF);
            currentPPCB->UseDepthOfF = useDOF;

            ImGui::InputFloat("DOF Far", &currentPPCB->FarPlane);
            if (currentPPCB->FarPlane < 0) {
                currentPPCB->FarPlane = 0;
            }


            ImGui::InputFloat("DOF Foacal width", &currentPPCB->focalwidth);
            ImGui::InputFloat("DOF Focal Dis", &currentPPCB->focalDistance);
            ImGui::InputFloat("DOF Attuenation", &currentPPCB->blerAttenuation);

            bool useblue= currentPPCB->UseBlur;

            ImGui::Checkbox("Blur", &useblue);
            currentPPCB->UseBlur = useblue;


            ImGui::Checkbox("RTT", &rtt);

            ImGui::SliderFloat("FadeLevel", &currentPPCB->fadeAmount, 0.0f, 1.0f,"%.3f");


        }
        if (ImGui::CollapsingHeader("Raster Select"))
        {

            if (ImGui::BeginCombo("##RScombo", current_RasterState.c_str())) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < RasterState->GetStateNames().size(); n++)
                {
                    bool is_selected = (current_RasterState == RasterState->GetStateNames()[n].c_str()); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(RasterState->GetStateNames()[n].c_str(), is_selected)) {

                        RasterState->SetState(RasterState->GetStateNames()[n].c_str());
                        current_RasterState = RasterState->GetStateNames()[n].c_str();
                    }

                }
                ImGui::EndCombo();
            }
        }
    }
    ImGui::End();
}


//objec control menue
static float rotationX, rotationY , rotationZ;
static float pos[]= { 0.0f,0.0f,0.0f };
void ImGuiManager::ObjectControl(DrawableGameObject* GameObject)
{
    if (ImGui::Begin("object Controll", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::CollapsingHeader("contolls"))
        {
            ImGui::SliderFloat("Rotation X",&rotationX,  0, 360);
            ImGui::SliderFloat("Rotation Y", &rotationY, 0, 360);
            ImGui::SliderFloat("Rotation Z", &rotationZ, 0, 360);
            GameObject->GetTransfrom()->SetRotation(rotationX, rotationY, rotationZ);


            ImGui::Text("Position");
            ImGui::InputFloat("X", &pos[0]);
            ImGui::InputFloat("Y", &pos[1]);
            ImGui::InputFloat("Z", &pos[2]);
            GameObject->GetTransfrom()->SetPosition(pos[0], pos[1], pos[2]);


            if (ImGui::Button("Reset")) {
                rotationX = 0.0f;
                rotationY = 0.0f;
                rotationZ = 0.0f;
                pos[0] = 0.0f;
                pos[1] = 0.0f;
                pos[2] = 0.0f;
            }
        }
        if (ImGui::CollapsingHeader("Tex Controll"))
        {

            MaterialPropertiesConstantBuffer data= GameObject->GetAppearance()->getMaterialPropertiesConstantBuffer();

            bool booldata = data.Material.UseTexture;
            ImGui::Text("Texture");
            ImGui::Checkbox("On", &booldata);

            ImGui::Text("Parralax Options");
            ImGui::InputFloat("Hight Scale",  &data.Material.HightScale, 0.00f, 0.0f, "%.2f");
            ImGui::InputFloat("Max Layer", &data.Material.MaxLayers);
            ImGui::InputFloat("Min Layer", &data.Material.MinLayers);


            data.Material.UseTexture = booldata;

            ImGui::Text("Diffuse");
            ImGui::InputFloat("dR", &data.Material.Diffuse.x);
            ImGui::InputFloat("dG", &data.Material.Diffuse.y);
            ImGui::InputFloat("dB", &data.Material.Diffuse.z);


            ImGui::Text("Specular");
            ImGui::InputFloat("sR", &data.Material.Specular.x);
            ImGui::InputFloat("sG", &data.Material.Specular.y);
            ImGui::InputFloat("sB", &data.Material.Specular.z);
            ImGui::InputFloat("power", &data.Material.SpecularPower);

            ImGui::Text("emissive");
            ImGui::InputFloat("eR", &data.Material.Emissive.x);
            ImGui::InputFloat("eG", &data.Material.Emissive.y);
            ImGui::InputFloat("eB", &data.Material.Emissive.z);


            ImGui::Text("ambient");
            ImGui::InputFloat("aR", &data.Material.Ambient.x);
            ImGui::InputFloat("aG", &data.Material.Ambient.y);
            ImGui::InputFloat("aB", &data.Material.Ambient.z);


            GameObject->GetAppearance()->SetMaterial(data);
        }
    }
    ImGui::End();
}

//light control menu
static const char* current_item1 = NULL;
static string name1;
static bool Load2 = false;

static Light CurrLightData;

void ImGuiManager::LightControl(LightController* LightControl)
{

    if (!Load2) {
        name1 = LightControl->GetLight(0)->GetName();
        CurrLightData = LightControl->GetLightList()[0]->GetLightData();
        current_item1 = name1.c_str();
        Load2 = true;
    }

    if (ImGui::Begin("Light Control", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::CollapsingHeader("contolls"))
        {
            if (ImGui::BeginCombo("##combo", current_item1)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < LightControl->GetLightList().size(); n++)
                {
                    bool is_selected = (current_item1 == LightControl->GetLightList()[n]->GetName().c_str()); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(LightControl->GetLightList()[n]->GetName().c_str(), is_selected)) {

                        name1 = LightControl->GetLightList()[n]->GetName().c_str();


                        CurrLightData = LightControl->GetLightList()[n]->GetLightData();
                        current_item1 = name1.c_str();
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)

                    }

                }
                ImGui::EndCombo();
            }

            CurrLightData = LightControl->GetLight(name1)->GetLightData();

            ImGui::Text("Position");
            ImGui::InputFloat("X", &CurrLightData.Position.x);
            ImGui::InputFloat("Y", &CurrLightData.Position.y);
            ImGui::InputFloat("Z", &CurrLightData.Position.z);


            bool enable = CurrLightData.Enabled;
            ImGui::Checkbox("Enabled", &enable);
            CurrLightData.Enabled = enable;

            float Colour[] = { CurrLightData.Color.x ,CurrLightData.Color.y,CurrLightData.Color.z,CurrLightData.Color.w };
            ImGui::ColorPicker4("Colour", Colour,ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB);
            CurrLightData.Color = { Colour[0],Colour[1],Colour[2],Colour[3] };

            ImGui::Text("Shdows Direction");

            XMFLOAT3 LightDirection = LightControl->GetLight(current_item1)->CamLight->GetRot();

           ImGui::SliderAngle("Pitch", &LightDirection.x, 0.995f * -90.0f, 0.995f * 90.0f);
           ImGui::SliderAngle("Yaw", &LightDirection.y, -180.0f, 180.0f);

            LightControl->GetLight(current_item1)->CamLight->SetRot(LightDirection);

            switch (LightControl->GetLight(name1)->GetLightData().LightType)
            {

            case LightType::PointLight:
                ImGui::Text("attenuation");
                ImGui::SliderFloat("Constant", &CurrLightData.ConstantAttenuation, 1.0f, 10.0f, "%.2f");
                ImGui::SliderFloat("Linear", &CurrLightData.LinearAttenuation, 0.0f, 5.0f, "%.4f");
                ImGui::SliderFloat("Quadratic", &CurrLightData.QuadraticAttenuation, 0.0f, 2.0f, "%.7f");
                break;

            case LightType::SpotLight:
            {
                ImGui::Text("attenuation");
                ImGui::SliderFloat("Constant", &CurrLightData.ConstantAttenuation, 1.0f, 10.0f, "%.2f");
                ImGui::SliderFloat("Linear", &CurrLightData.LinearAttenuation, 0.0f, 5.0f, "%.4f");
                ImGui::SliderFloat("Quadratic", &CurrLightData.QuadraticAttenuation, 0.0f, 2.0f, "%.7f");

                float SpotAngle = XMConvertToDegrees(CurrLightData.SpotAngle);
                ImGui::InputFloat("Spot Angle", &SpotAngle);
                CurrLightData.SpotAngle = XMConvertToRadians(SpotAngle);

            }
                break;

            case LightType::DirectionalLight:
                ImGui::Text("Direction");
                ImGui::InputFloat("A", &CurrLightData.Direction.x);
                ImGui::InputFloat("B", &CurrLightData.Direction.y);
                ImGui::InputFloat("C", &CurrLightData.Direction.z);


                break;

            default:
                break;
            }


            LightControl->GetLight(name1)->SetLightData(CurrLightData);


        }

    }
    ImGui::End();
}


//billboard controll menu
static const char* current_item_Bill= NULL;
static string nameBill;
//static bool Load2 = false;
static vector<SimpleVertexBill> billpos;
static int picked=0;
void ImGuiManager::BillBoradControl(BillboardObject* BillControl)
{
    billpos = BillControl->GetPosistions();
    if (ImGui::Begin("BillBoard Control", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::BeginCombo("##combo", current_item_Bill)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < BillControl->GetPosistions().size(); n++)
            {
                string name = "Billboard ";
                name+= to_string(n);
                bool is_selected = (current_item_Bill == name.c_str()); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(name.c_str(), is_selected)) {

                    nameBill = name;


                    billpos = BillControl->GetPosistions();
                    picked = n;
                    current_item_Bill = nameBill.c_str();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                    billpos = BillControl->GetPosistions();
                }

            }
            ImGui::EndCombo();

        }
        ImGui::Text("Pos");
            ImGui::InputFloat("x", &billpos[picked].Pos.x);
            ImGui::InputFloat("y", &billpos[picked].Pos.y);
            ImGui::InputFloat("z", &billpos[picked].Pos.z);

            BillControl->SetPositions(billpos);


    }
    ImGui::End();


}

void ImGuiManager::BezierCurveSpline()
{
    if (ImGui::Begin("BezierCurveSpline", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {
        float lines[120];
        for (int n = 0; n < points.size(); n++)
            lines[n] = points[n].y;
        ImGui::PlotLines("G", lines, points.size());


    }
    ImGui::End();
}


void ImGuiManager::TerrainControll(Terrain* terrain, TerrainVoxel* VoxelTerrain, ID3D11Device* device, ID3D11DeviceContext* _pImmediateContext)
{
    if (ImGui::Begin("Terrain", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {

        if (ImGui::CollapsingHeader("Terrain"))
        {
            ImGui::Checkbox("Draw terrain", terrain->GetIsDraw());
            ImGui::Text("Terrain Data");

            ImGui::Text("width = %i", terrain->GetHightMapWidth());
            ImGui::Text("Hight = %i", terrain->GetHightMapHight());
            ImGui::Text("Cell Spacing = %f", terrain->GetCellSpacing());
            switch (terrain->GetGenType())
            {
            case TerrainGenType::HightMapLoad:
                ImGui::Text("GenType: HightMap");
                ImGui::Text(terrain->GetHightMapName().c_str());
                ImGui::Text("Hight Scale = %i", terrain->GetHightScale());
                break;
            case TerrainGenType::FaultLine:
                ImGui::Text("GenType: FaultLine");
                ImGui::Text("Seed:  %i", terrain->GetSeed());
                ImGui::Text("Displacment:  %f", terrain->GetDisplacment());
                ImGui::Text("Number Of iterations:  %i", terrain->GetNumberOfIterations());

                break;
            case TerrainGenType::Noise:
                ImGui::Text("GenType: Noise");
                ImGui::Text("Hight Scale = %i", terrain->GetHightScale());
                ImGui::Text("Seed:  %i", terrain->GetSeed());
                ImGui::Text("Fequancy:  %f", terrain->GetFequancy());
                ImGui::Text("Number of ocatives:  %i", terrain->GetNumberOfOcatives());
                break;
            case TerrainGenType::DiamondSquare:
                ImGui::Text("GenType: Diamond Square");
                ImGui::Text("Hight Scale = %i", terrain->GetHightScale());
                ImGui::Text("Seed:  %i", terrain->GetSeed());
                ImGui::Text("Range:  %i", terrain->GetRange());
                break;
            default:
                break;
            }

            if (ImGui::CollapsingHeader("Build Data"))
            {
                //mode
                static TerrainGenType Mode= (TerrainGenType)0;
                const char* items[] = { "HightMapLoad", "FaultLine", "Noise", "DiamondSquare" };
                static const char* current_item = "HightMapLoad";

                if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(items[n], is_selected)) {
                            current_item = items[n];
                            Mode = (TerrainGenType)n;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                        }
                    }
                    ImGui::EndCombo();
                }

                static int width= 514;
                ImGui::InputInt("width", &width);
                static int Length= 514;
                ImGui::InputInt("Length", &Length);

                static float CellSpaceing=1.0f;
                ImGui::InputFloat("CellSpaceing", &CellSpaceing);

                static float HightScale=50.0f;
                static float Frequancy = 0.01f;
                static float Displacement = 0.01f;
                static int seed=1234;
                static int Range = 196;
                static int NumberOfOctives = 3;
                switch (Mode)
                {
                case TerrainGenType::HightMapLoad:
                    ImGui::InputFloat("HightScale", &HightScale);
                    break;
                case TerrainGenType::FaultLine:
                    ImGui::InputInt("Seed", &seed);
                    ImGui::InputInt("NumberOfIteration", &Range);
                    ImGui::InputFloat("Displacement", &Displacement);
                    break;
                case TerrainGenType::Noise:
                    ImGui::InputInt("Seed", &seed);
                    ImGui::InputFloat("Frequancy", &Frequancy);
                    ImGui::InputInt("NumberOfOctives", &NumberOfOctives);
                    ImGui::InputFloat("HightScale", &HightScale);
                    break;
                case TerrainGenType::DiamondSquare:
                    ImGui::InputInt("Seed", &seed);
                    ImGui::InputInt("Range", &Range);
                    ImGui::InputFloat("HightScale", &HightScale);
                    break;
                default:
                    break;
                }

                if (ImGui::Button("Build Terrain")) {
                        switch (Mode)
                        {
                        case TerrainGenType::HightMapLoad:
                            break;
                        case TerrainGenType::FaultLine:
                            terrain->SetFualtLineData(seed, Range, Displacement);
                            break;
                        case TerrainGenType::Noise:
                            terrain->SetNoiseData(seed, Frequancy, NumberOfOctives);
                            break;
                        case TerrainGenType::DiamondSquare:
                            terrain->SetDimondSquaerData(seed, Range);
                            break;
                        default:
                            break;
                        }

                    terrain->ReBuildTerrain(XMFLOAT2(width, Length), HightScale, CellSpaceing, Mode, device);


                }
            }
            if (ImGui::CollapsingHeader("Textuer Data"))
            {
                ImGui::BeginTable("Textuer Name", 3);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Name");


                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Water");

                for (size_t i = 0; i < terrain->GetTexNames().size() ; i++)
                {
                    string data = terrain->GetTexNames()[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text( data.c_str());

                }
                ImGui::EndTable();

                ImGui::Text("Textuer Hight Level");

                float Layer1MaxHight= terrain->GetTerrainData().Layer1MaxHight;
                float Layer2MaxHight= terrain->GetTerrainData().Layer2MaxHight;
                float Layer3MaxHight= terrain->GetTerrainData().Layer3MaxHight;
                float Layer4MaxHight= terrain->GetTerrainData().Layer4MaxHight;
                float Layer5MaxHight= terrain->GetTerrainData().Layer5MaxHight;

                ImGui::SliderFloat("Layer1MaxHight", &Layer1MaxHight, 0.0f, Layer2MaxHight);
                ImGui::SliderFloat("Layer2MaxHight", &Layer2MaxHight, Layer1MaxHight, Layer3MaxHight);
                ImGui::SliderFloat("Layer3MaxHight", &Layer3MaxHight, Layer2MaxHight, Layer4MaxHight);
                ImGui::SliderFloat("Layer4MaxHight", &Layer4MaxHight, Layer3MaxHight, Layer5MaxHight);
                ImGui::SliderFloat("Layer5MaxHight", &Layer5MaxHight, Layer4MaxHight, 100000.0f);
                terrain->SetTexHights(Layer1MaxHight, Layer2MaxHight, Layer3MaxHight, Layer4MaxHight, Layer5MaxHight);

            }
            if (ImGui::CollapsingHeader("LOD Data"))
            {
                float FloatMinTess= terrain->GetTerrainData().MinTess;
                float FloatMaxTess= terrain->GetTerrainData().MaxTess;
                ImGui::SliderFloat("minTess", &FloatMinTess,0.0f,6.0f);
                ImGui::SliderFloat("maxTess", &FloatMaxTess, 0.0f, 6.0f);
                if (FloatMaxTess < FloatMinTess) {
                    FloatMaxTess = FloatMinTess;
                }
                terrain->SetMaxTess(FloatMaxTess);
                terrain->SetMinTess(FloatMinTess);

                float maxTessDist= terrain->GetTerrainData().MaxDist;
                float minTessDist= terrain->GetTerrainData().MinDist;
                ImGui::SliderFloat("Min Tess Dist", &minTessDist, 1.0f, 1000.0f);
                ImGui::SliderFloat("Max Tess Dist", &maxTessDist, 1.0f, 1000.0f);
                if (maxTessDist < minTessDist) {
                    maxTessDist = minTessDist;
                }
                terrain->SetMaxTessDist(maxTessDist);
                terrain->SetMinTessDist(minTessDist);
            }
            if (ImGui::CollapsingHeader("Transfrom Data"))
            {
                ImGui::Text("Rotation");
                XMFLOAT3 RotationTerrain = terrain->GetTransfrom()->GetRotation();
                ImGui::SliderFloat("Rotation X", &RotationTerrain.x, 0, 360);
                ImGui::SliderFloat("Rotation Y", &RotationTerrain.y, 0, 360);
                ImGui::SliderFloat("Rotation Z", &RotationTerrain.z, 0, 360);
                terrain->GetTransfrom()->SetRotation(RotationTerrain);

                ImGui::Text("Position");
                XMFLOAT3 posTerrain = terrain->GetTransfrom()->GetPosition();
                ImGui::InputFloat("Position X", &posTerrain.x);
                ImGui::InputFloat("Position Y", &posTerrain.y);
                ImGui::InputFloat("Position Z", &posTerrain.z);
                terrain->GetTransfrom()->SetPosition(posTerrain);

                ImGui::Text("Scale");
                XMFLOAT3 scaleTerrain = terrain->GetTransfrom()->GetScale();
                ImGui::InputFloat("Scale X", &scaleTerrain.x);
                ImGui::InputFloat("Scale Y", &scaleTerrain.y);
                ImGui::InputFloat("Scale Z", &scaleTerrain.z);
                terrain->GetTransfrom()->SetScale(scaleTerrain);
            }


             static string FileName;
            //ImGui::InputText("FileName", &FileName);
            if (ImGui::Button("Load")) {

                TerrainData Data;
                TerrainJsonLoad::LoadData(FileName, Data);
                TerrainGenType GenMode = (TerrainGenType)Data.mode;
                double HightScale = 0;
                switch (GenMode)
                {
                case TerrainGenType::HightMapLoad:
                    HightScale = Data._HightMapSettings.HightScale;
                    break;
                case TerrainGenType::FaultLine:
                    terrain->SetFualtLineData(Data._FaultLineSettings.Seed, Data._FaultLineSettings.iterationCount, Data._FaultLineSettings.Displacement);
                    HightScale = terrain->GetHightScale();
                    break;
                case TerrainGenType::Noise:
                    terrain->SetNoiseData(Data._NoiseSettings.Seed, Data._NoiseSettings.Frequancy, Data._NoiseSettings.NumberOfOctaves);
                    HightScale = Data._NoiseSettings.HightScale;
                    break;
                case TerrainGenType::DiamondSquare:
                    terrain->SetDimondSquaerData(Data._DimondSquareSettings.Seed, Data._DimondSquareSettings.range);
                    HightScale = Data._DimondSquareSettings.HightScale;
                    break;
                default:
                    break;
                }

                terrain->ReBuildTerrain(XMFLOAT2(Data.Width, Data.Depth), HightScale, Data.CellSpaceing, GenMode, device);
            }


            if (ImGui::Button("Save")) {

                TerrainData Data;
                Data.Width = terrain->GetHightMapWidth();
                Data.Depth = terrain->GetHightMapHight();
                Data.CellSpaceing = terrain->GetCellSpacing();
                Data.mode = (int)terrain->GetGenType();

                switch (terrain->GetGenType())
                {
                case TerrainGenType::HightMapLoad:
                    Data._HightMapSettings.HightMapFile = terrain->GetHightMapName();
                    Data._HightMapSettings.HightScale = terrain->GetHightScale();
                    break;
                case TerrainGenType::FaultLine:
                    Data._FaultLineSettings.Seed = terrain->GetSeed();
                    Data._FaultLineSettings.iterationCount = terrain->GetNumberOfIterations();
                    Data._FaultLineSettings.Displacement = terrain->GetDisplacment();
                    break;
                case TerrainGenType::Noise:
                    Data._NoiseSettings.Seed = terrain->GetSeed();
                    Data._NoiseSettings.HightScale = terrain->GetHightScale();
                    Data._NoiseSettings.Frequancy = terrain->GetFequancy();
                    Data._NoiseSettings.NumberOfOctaves = terrain->GetNumberOfOcatives();
                    break;
                case TerrainGenType::DiamondSquare:
                    Data._DimondSquareSettings.Seed = terrain->GetSeed();
                    Data._DimondSquareSettings.HightScale = terrain->GetHightScale();
                    Data._DimondSquareSettings.range = terrain->GetRange();
                    break;

                }

                TerrainJsonLoad::StoreData(FileName, Data);
            }

        }
        if (ImGui::CollapsingHeader("Voxel Terrain"))
        {
            ImGui::Checkbox("Draw Voxel", VoxelTerrain->GetIsDraw());
            ImGui::Text("Number Of Chunks: %i", VoxelTerrain->GetNumberOfChunks());

            if (ImGui::CollapsingHeader("Rebuild World"))
            {
                static int seed;
                ImGui::InputInt("Seed Vox", &seed);
                static float Frequancy;
                ImGui::InputFloat("Frequancy Vox", &Frequancy);
                static int Octave;
                ImGui::InputInt("Octave Vox", &Octave);
                static int NumberOfChuncksX = 0;
                ImGui::InputInt("Number of chunk x", &NumberOfChuncksX);
                static int NumberOfChuncksZ = 0;
                ImGui::InputInt("Number of chunk z", &NumberOfChuncksZ);
                if (ImGui::Button("Build")) {

                    VoxelTerrain->RebuildMap(device, _pImmediateContext, seed, NumberOfChuncksX, NumberOfChuncksZ,Frequancy,Octave);



                }
            }

        }
    }
    ImGui::End();
}

void ImGuiManager::AnimationControll(AnimatedModel* AnnimationModel)
{
    if (ImGui::Begin("Annimation", FALSE, ImGuiWindowFlags_AlwaysAutoResize))
    {

        if (ImGui::CollapsingHeader("Model Data"))
        {
            std::string ModelName = "Model Name ";
            ModelName += AnnimationModel->GetModelName().c_str();
            ImGui::Text(ModelName.c_str());
            ImGui::BeginTable("SubSetData", 5);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("ID");
            ImGui::TableNextColumn();
            ImGui::Text("FaceCount");
            ImGui::TableNextColumn();
            ImGui::Text("VertexCount");
            ImGui::TableNextColumn();
            ImGui::Text("Diffuse Tex");
            ImGui::TableNextColumn();
            ImGui::Text("Normal Map");
                for (size_t i = 0; i < AnnimationModel->GetSubsets().size(); i++)
                {
                    Subset data   =  AnnimationModel->GetSubsets()[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%i", data.Id);
                    ImGui::TableNextColumn();
                    ImGui::Text("%i", data.FaceCount);
                    ImGui::TableNextColumn();
                    ImGui::Text("%i", data.VertexCount);
                    ImGui::TableNextColumn();
                    wstring DiffMapA = AnnimationModel->GetMaterrialData()[i].DiffuseMapName.c_str();
                    ImGui::Text(StringHelpers::ws2s(DiffMapA).c_str());
                    ImGui::TableNextColumn();
                    wstring normMapA = AnnimationModel->GetMaterrialData()[i].NormalMapName.c_str();
                    ImGui::Text(StringHelpers::ws2s(normMapA).c_str());
                }
            ImGui::EndTable();
            ImGui::Text("Model Place");

            static float ModelPos[3] = {
                AnnimationModel->GetTransformData()->GetPosition().x,
                AnnimationModel->GetTransformData()->GetPosition().y,
                AnnimationModel->GetTransformData()->GetPosition().z
            };
            ImGui::InputFloat3("Model Position", ModelPos);
            AnnimationModel->GetTransformData()->SetPosition(XMFLOAT3(ModelPos));
            static float ModelScale[3] = {
                AnnimationModel->GetTransformData()->GetScale().x,
                AnnimationModel->GetTransformData()->GetScale().y,
                AnnimationModel->GetTransformData()->GetScale().z
            };
            ImGui::InputFloat3("Model Scale", ModelScale);
            AnnimationModel->GetTransformData()->SetScale(XMFLOAT3(ModelScale));
            static float ModelRotation[3] = {
                AnnimationModel->GetTransformData()->GetRotation().x,
                AnnimationModel->GetTransformData()->GetRotation().y,
                AnnimationModel->GetTransformData()->GetRotation().z
            };
            ImGui::InputFloat3("Model Rotation", ModelRotation);
            AnnimationModel->GetTransformData()->SetRotation(XMFLOAT3(ModelRotation));



        }
        if (ImGui::CollapsingHeader("Bone Data"))
        {
            if (ImGui::BeginChild("BoneList", ImVec2(500, 100), true, ImGuiWindowFlags_HorizontalScrollbar)) {
                ImGui::BeginTable("BoneInfo", 3);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("ID");
                ImGui::TableNextColumn();
                ImGui::Text("Parent");
                ImGui::TableNextColumn();
                ImGui::Text("Child");

                for (size_t i = 0; i < AnnimationModel->GetSkeleton()->GetBoneData().size(); i++)
                {
                    Bone* data = AnnimationModel->GetSkeleton()->GetBoneData()[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%i", i);
                    ImGui::TableNextColumn();
                    ImGui::Text("%i", data->Getparent());
                    ImGui::TableNextColumn();
                    string childData = "";
                    for (size_t i = 0; i < data->GetChild().size(); i++)
                    {
                        if (data->GetChild()[i]==-1) {
                            childData = "No Child";
                        }
                        else if (i == data->GetChild().size() - 1) {
                            childData += to_string(data->GetChild()[i]);
                        }
                        else
                        {
                            childData += to_string(data->GetChild()[i]) + ", ";

                        }
                    }
                    ImGui::Text(childData.c_str());

                }
                ImGui::EndTable();
            }
            ImGui::EndChild();

            ImGui::Text("Edit Bone");
            static int BoneNum = 0;
            static int BoneNumPrev = -1;
            static float BonePos[3];
            static float BoneScale[3];

            ImGui::InputInt("Bone No:", &BoneNum);

            if (BoneNum > AnnimationModel->GetSkeleton()->BoneCount()) {
                BoneNum = AnnimationModel->GetSkeleton()->BoneCount() - 1;
            }
            else if (BoneNum < 0) {
                BoneNum = 0;
            }
            if (BoneNum != BoneNumPrev) {
                BonePos[0] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldPos().x;
                BonePos[1] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldPos().y;
                BonePos[2] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldPos().z;

                BoneScale[0] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldScale().x;
                BoneScale[1] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldScale().y;
                BoneScale[2] = AnnimationModel->GetSkeleton()->GetBoneData()[BoneNum]->GetWorldScale().z;
                BoneNumPrev = BoneNum;
            }


            ImGui::InputFloat3("Bone Position", BonePos);
            ImGui::InputFloat3("Bone Scale", BoneScale);

            static float BoneRotation[3];
            ImGui::InputFloat3("Bone Rotation", BoneRotation);

            if (ImGui::Button("SetBone")) {
                AnnimationModel->GetSkeleton()->SetBonePosition(BoneNum,XMFLOAT3(BonePos));

                XMMATRIX rotation = XMMatrixRotationX(XMConvertToRadians(BoneRotation[0])) * XMMatrixRotationY(XMConvertToRadians(BoneRotation[1])) * XMMatrixRotationZ(XMConvertToRadians(BoneRotation[2]));
                XMFLOAT4 orientation;
                XMStoreFloat4(&orientation, XMQuaternionRotationMatrix(rotation));
                AnnimationModel->GetSkeleton()->SetBoneRotQuat(BoneNum,XMFLOAT4(orientation));

                AnnimationModel->GetSkeleton()->SetBoneScale(BoneNum,XMFLOAT3(BoneScale));
            }

        }
        if (ImGui::CollapsingHeader("Annimation Data"))
        {
            //set animation
            static std::vector<std::string> AnimationName = AnnimationModel->GetSkeleton()->mAnimationClips();

            static const char* current_item2 = AnimationName[0].c_str();

            if (ImGui::BeginCombo("##combo", current_item2)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < AnimationName.size(); n++)
                {
                    bool is_selected = (current_item2 == AnimationName[n].c_str()); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(AnimationName[n].c_str(), is_selected)) {
                        current_item2 = AnimationName[n].c_str();

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                            AnnimationModel->SetAnnimation(AnimationName[n]);
                        }
                    }
                }
                ImGui::EndCombo();
            }
            AnnimationModel->SetAnnimation(current_item2);
            static bool isLoop = AnnimationModel-> GetIsLoop();
            ImGui::Checkbox("Loop Animation", &isLoop);
            AnnimationModel->SetIsLoop(isLoop);
            float TimePos= AnnimationModel->GetTimePos();
            if (!isLoop) {

                ImGui::InputFloat("Animtion Time position", &TimePos);
                AnnimationModel->SetTimePos(TimePos);
            }

            //current animation
            ImGui::Text("Current Animation: %s", AnnimationModel->GetClipName().c_str());
            ImGui::Text("Current Animation Start: %f", AnnimationModel->GetSkeleton()->GetClipStartTime(AnnimationModel->GetClipName()));
            ImGui::Text("Current Animation End: %f", AnnimationModel->GetSkeleton()->GetClipEndTime(AnnimationModel->GetClipName()));
            ImGui::Text("Current Time: %f", AnnimationModel->GetTimePos());
        }

    }
    ImGui::End();
}

void ImGuiManager::SetBlackGoldStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style->FramePadding = ImVec2(4, 2);
    style->ItemSpacing = ImVec2(10, 2);
    style->IndentSpacing = 12;
    style->ScrollbarSize = 10;

    style->WindowRounding = 4;
    style->FrameRounding = 4;
    style->PopupRounding = 4;
    style->ScrollbarRounding = 6;
    style->GrabRounding = 4;
    style->TabRounding = 4;

    style->WindowTitleAlign = ImVec2(1.0f, 0.5f);
    style->WindowMenuButtonPosition = ImGuiDir_Right;

    style->DisplaySafeAreaPadding = ImVec2(4, 4);
}