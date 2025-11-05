#include <QString>
#include <QFile>

#include "Xenoverse2.h"
#include "Config.h"
#include "xv2ins_common.h"
#include "CharaSeleIggyBlob.h"
#include "stageseleiggyblob.h"
#include "popuppauseiggyblob.h"
#include "itemlistdetailsblob.h"

//#define MY_CHARALIST_OLD        "XV2INS\\CharaListV6.as"
//#define MY_CHARALIST_NEW        "data/XV2P_SLOTS.x2s"
#define CHARASELE_IGGY_PATH     "data/ui/iggy/CHARASELE.iggy"
#define STAGESELE_IGGY_PATH     "data/ui/iggy/STAGESELE.iggy"
#define POPUPPAUSE_IGGY_PATH     "data/ui/iggy/POPUP_PAUSE.iggy"
#define ILD_IGGY_PATH            "data/ui/iggy/ITEMLIST_DETAILS.iggy"
//#define CHASEL_PATH "Internal/CharaSele"

#define GAME_CST_FILE   "data/system/chara_select_table.cst"

static bool IsOldStyleCus()
{
    CusFile cus;

    if (!xv2fs->LoadFile(&cus, "data/system/custom_skill.cus"))
        return false;

    return (cus.GetVersion() < 125);
}

static bool NeedsUpdate()
{
    return IsOldStyleCus();
}

static uint8_t *ReadResourceFile(const char *path, size_t *size)
{
    QFile file(path);

     if (!file.open(QFile::ReadOnly))
         return nullptr;

     uint8_t *buf = new uint8_t[file.size()];
     if (!buf)
     {
         DPRINTF("%s: Memory allocation error: %x\n", FUNCNAME, (unsigned int)file.size());
         file.close();
         return nullptr;
     }

     if (file.read((char *)buf, file.size()) != file.size())
     {
         delete[] buf;
         file.close();
         return nullptr;
     }

     *size = file.size();
     file.close();

     return buf;
}

static void InstallIggy(const char *path, const uint8_t *blob, size_t blob_size, const char *tag)
{
    bool install_iggy = true;

    if (xv2fs->FileExists(path, false))
    {
        size_t size;
        uint8_t *buf = xv2fs->ReadFile(path, &size);

        if (!buf)
        {
            DPRINTF("%s: Failed to load %s\n", FUNCNAME, path);
            exit(-1);
        }

        for (size_t i = 0; i < size-strlen(tag)-1; i++)
        {
            if (memcmp(buf+i, tag, strlen(tag)) == 0)
            {
                install_iggy = false;
                break;
            }
        }

        delete[] buf;
    }

    if (install_iggy)
    {
        if (!xv2fs->WriteFile(path, blob, blob_size))
        {
            DPRINTF("%s: Failed to save %s\n", FUNCNAME, path);
            exit(-1);
        }
    }
}

bool Bootstrap(bool multiple_hci, bool installer_mode, bool *needs_update)
{
    if (needs_update)
        *needs_update = false;

    config.Load();

    if (!config.ConfigureRequirements())
    {
        DPRINTF("ConfigureRequirements failed.\n");
        exit(-1);
    }

    Xenoverse2::InitFs(Utils::QStringToStdString(config.game_directory));

    config.LanguageSetup(installer_mode);

    float version = Utils::GetExeVersion(Utils::MakePathString(Utils::QStringToStdString(config.game_directory), "bin/DBXV2.exe"));
    if (version < (MINIMUM_EXE_VERSION_REQUIRED-0.0001))
    {
        DPRINTF("Version %.3f or greater of the game is required. You have version %.3f.\n", MINIMUM_EXE_VERSION_REQUIRED, version);
        return false;
    }

    if (needs_update)
    {
        *needs_update = NeedsUpdate();

        if (*needs_update)
            return false;
    }

    InstallIggy(CHARASELE_IGGY_PATH, charasele_iggy_blob, sizeof(charasele_iggy_blob), XV2_PATCHER_AS3_TAG_CHARA);
    InstallIggy(STAGESELE_IGGY_PATH, stagesele_iggy_blob, sizeof(stagesele_iggy_blob), XV2_PATCHER_AS3_TAG_STAGE);
    InstallIggy(POPUPPAUSE_IGGY_PATH, popuppause_iggy_blob, sizeof(popuppause_iggy_blob), XV2_PATCHER_AS3_TAG_PAUSE);
    InstallIggy(ILD_IGGY_PATH, itemlist_details_iggy_blob, sizeof(itemlist_details_iggy_blob), XV2_PATCHER_AS3_TAG_ILD);

    //if (!Xenoverse2::InitCharaList(CHASEL_PATH, my_charalist))
    if (!Xenoverse2::InitCharaList())
    {
        DPRINTF("InitCharaList failed.\n");
        exit(-1);
    }    

    /*if (!config.prb_fix_applied)
    {
        config.prb_fix_applied = true;
        config.Save();

        size_t num_slots = chara_list->GetNumSlots();
        if ((*chara_list)[num_slots-1].entries[0].code != "MRN")
        {
            // Zamasu damaged
            {
                CharaListSlot c_slot;
                CharaListSlotEntry s_entry;

                s_entry.costume_index = 0;
                s_entry.code = "ZMD";
                s_entry.code = "\"" + s_entry.code + "\"";
                s_entry.model_preset = 1;
                s_entry.flag_gk2 = false;
                s_entry.unlock_index = 0;
                s_entry.voices_id_list[0] = 178;
                s_entry.voices_id_list[1] = 179;
                s_entry.dlc = "PRB";

                c_slot.entries.push_back(s_entry);
                chara_list->GetSlots().push_back(c_slot);
            }

            // Demigra
            {
                CharaListSlot c_slot;
                CharaListSlotEntry s_entry;

                s_entry.costume_index = 0;
                s_entry.code = "DMG";
                s_entry.code = "\"" + s_entry.code + "\"";
                s_entry.model_preset = 1;
                s_entry.flag_gk2 = false;
                s_entry.unlock_index = 0;
                s_entry.voices_id_list[0] = 209;
                s_entry.voices_id_list[1] = 210;
                s_entry.dlc = "PRB";

                c_slot.entries.push_back(s_entry);
                chara_list->GetSlots().push_back(c_slot);
            }

            // Demigra
            {
                CharaListSlot c_slot;
                CharaListSlotEntry s_entry;

                s_entry.costume_index = 0;
                s_entry.code = "MRN";
                s_entry.code = "\"" + s_entry.code + "\"";
                s_entry.model_preset = 1;
                s_entry.flag_gk2 = false;
                s_entry.unlock_index = 0;
                s_entry.voices_id_list[0] = 142;
                s_entry.voices_id_list[1] = 143;
                s_entry.dlc = "PRB";

                c_slot.entries.push_back(s_entry);
                chara_list->GetSlots().push_back(c_slot);
            }

            Xenoverse2::CommitCharaList(true, false);
        }
    }*/

    if (!Xenoverse2::InitSystemFiles(false, multiple_hci))
    {
        DPRINTF("InitSystemFiles failed.\n");

        if (!installer_mode)
            exit(-1);

        return false;
    }

    if (needs_update && game_cus->GetVersion() < 121)
    {
        *needs_update = true;
        return false;
    }

    if (!Xenoverse2::InitCharaNames())
    {
        DPRINTF("InitCharaNames failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitCharaCostumeNames())
    {
        DPRINTF("InitCharaCostumeNames failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitSkillNames())
    {
        DPRINTF("InitSkillNames failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitSkillDescs())
    {
        DPRINTF("InitSkillDescs failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitBtlHudText())
    {
        DPRINTF("InitBtlHudText failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitSelPort())
    {
        DPRINTF("InitSelPort failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitPreBaked())
    {
        DPRINTF("InitPreBaked failed.\n");
        exit(-1);
    }

    // This is now loaded on demand
    /*if (!Xenoverse2::InitSound(true))
    {
        DPRINTF("InitSound failed.\n");
        exit(-1);
    }*/

    if (!Xenoverse2::InitIdb(true, true, true, true))
    {
        DPRINTF("InitIdb failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitTalismanNames())
    {
        DPRINTF("InitTalismanNames failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitCac())
    {
        DPRINTF("InitCac failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitCacCostumeNames())
    {
        DPRINTF("InitCacCostumeNames failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitCacCostumeDescs())
    {
        DPRINTF("InitCacCostumeDescs failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitCostumeFile())
    {
        DPRINTF("InitCostumeFile failed.\n");
        exit(-1);
    }

    if (!Xenoverse2::InitStageSlots())
    {
        DPRINTF("InitStageSlots failed.\n");
        exit(-1);
    }    

    if (!installer_mode)
        return true;    

    return true;
}


