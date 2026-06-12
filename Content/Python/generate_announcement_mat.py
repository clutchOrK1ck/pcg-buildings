import unreal

# ------------------------------------------------------------
# CONFIG
# ------------------------------------------------------------

PARENT_MATERIAL_PATH = "/Game/Materials/Master/M_Announcement"
TEXTURE_FOLDER = "/Game/Textures/Announcements"
OUTPUT_FOLDER = "/Game/Materials/Announcements"

TEXTURE_PARAMETER_NAME = "AnnouncementSurfaceTexture"

class Params:
    Texture = "AnnouncementSurfaceTexture"
    Cutoff = "Cutoff"
    NoiseIntensity = "NoiseIntensity"
    Rotation = "Rotation"
    RandomizeTabsPerInstance = "RandomizeTabsPerInstance"


DEFAULT_PARAMS = {
    'A4-landscape': {
        Params.Cutoff: -5.,
        Params.NoiseIntensity: 0.968,
        Params.Rotation: 20 / 360,
        Params.RandomizeTabsPerInstance: True
    },
    'A5-landscape': {
        Params.Cutoff: -5.,
        Params.NoiseIntensity: 0.968,
        Params.Rotation: 20 / 360,
        Params.RandomizeTabsPerInstance: True
    },
    'A5-portrait': {
        Params.Cutoff: -5.,
        Params.NoiseIntensity: 0.968,
        Params.Rotation: 20 / 360,
        Params.RandomizeTabsPerInstance: True
    },
    'small-1': {
        Params.Cutoff: -5.,
        Params.NoiseIntensity: 0.968,
        Params.Rotation: 20 / 360,
        Params.RandomizeTabsPerInstance: True
    },
    'small-2': {
        Params.Cutoff: -5.,
        Params.NoiseIntensity: 0.968,
        Params.Rotation: 20 / 360,
        Params.RandomizeTabsPerInstance: True
    }
}

# Only create instances for these texture classes.
VALID_TEXTURE_CLASSES = (
    unreal.Texture2D,
)


# ------------------------------------------------------------
# SETUP
# ------------------------------------------------------------

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary
material_editing_lib = unreal.MaterialEditingLibrary

parent_material = editor_asset_lib.load_asset(PARENT_MATERIAL_PATH)

if not parent_material:
    raise RuntimeError(f"Could not find parent material: {PARENT_MATERIAL_PATH}")

if not editor_asset_lib.does_directory_exist(TEXTURE_FOLDER):
    raise RuntimeError(f"Texture folder does not exist: {TEXTURE_FOLDER}")

if not editor_asset_lib.does_directory_exist(OUTPUT_FOLDER):
    editor_asset_lib.make_directory(OUTPUT_FOLDER)


# ------------------------------------------------------------
# FIND TEXTURES
# ------------------------------------------------------------

texture_asset_paths = editor_asset_lib.list_assets(
    TEXTURE_FOLDER,
    recursive=True,
    include_folder=False
)

textures = []

for asset_path in texture_asset_paths:
    asset = editor_asset_lib.load_asset(asset_path)

    if isinstance(asset, VALID_TEXTURE_CLASSES):
        textures.append(asset)

unreal.log(f"Found {len(textures)} textures in {TEXTURE_FOLDER}")


# ------------------------------------------------------------
# CREATE MATERIAL INSTANCES
# ------------------------------------------------------------

for texture in textures:
    texture_name = texture.get_name()

    # Example:
    # T_Brick_BaseColor -> MI_T_Brick_BaseColor
    instance_name = f"MI_Annon_{texture_name}"

    if editor_asset_lib.does_asset_exist(f'{OUTPUT_FOLDER}/{instance_name}'):
        continue

    factory = unreal.MaterialInstanceConstantFactoryNew()

    mi = asset_tools.create_asset(
        asset_name=instance_name,
        package_path=OUTPUT_FOLDER,
        asset_class=unreal.MaterialInstanceConstant,
        factory=factory,
    )

    unreal.log(f"Created material instance: {mi.get_path_name()}")

    # Make sure the instance has the correct parent.
    mi.set_editor_property("parent", parent_material)

    # Set texture parameter.
    material_editing_lib.set_material_instance_texture_parameter_value(
        mi,
        TEXTURE_PARAMETER_NAME,
        texture
    )

    # set the default parameters
    param_overrides = [overrides for tex_name, overrides in DEFAULT_PARAMS.items() if tex_name in texture_name][0]

    for param_name in param_overrides:
        param_value = param_overrides[param_name]

        setter = None
        if type(param_value) in {int, float}:
            setter = material_editing_lib.set_material_instance_scalar_parameter_value
        elif type(param_value) is bool:
            setter = material_editing_lib.set_material_instance_static_switch_parameter_value
        elif type(param_value) is unreal.LinearColor:
            setter = material_editing_lib.set_material_instance_vector_parameter_value

        if setter:
            setter(mi, param_name, param_value)

    # Update and save.
    material_editing_lib.update_material_instance(mi)
    editor_asset_lib.save_loaded_asset(mi)

unreal.log("Done creating texture-based material instances.")