#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#define STBTT_STATIC
#include <tesla.hpp>    // The Tesla Header
#include <string_view>
#include "minIni/minIni.h"

namespace {

constexpr auto CONFIG_PATH = "/config/sys-patch/config.ini";
constexpr auto LOG_PATH = "/config/sys-patch/log.ini";

auto does_file_exist(const char* path) -> bool {
    Result rc{};
    FsFileSystem fs{};
    FsFile file{};
    char path_buf[FS_MAX_PATH]{};

    if (R_FAILED(fsOpenSdCardFileSystem(&fs))) {
        return false;
    }

    strcpy(path_buf, path);
    rc = fsFsOpenFile(&fs, path_buf, FsOpenMode_Read, &file);
    fsFileClose(&file);
    fsFsClose(&fs);
    return R_SUCCEEDED(rc);
}

// creates a directory, non-recursive!
auto create_dir(const char* path) -> bool {
    Result rc{};
    FsFileSystem fs{};
    char path_buf[FS_MAX_PATH]{};

    if (R_FAILED(fsOpenSdCardFileSystem(&fs))) {
        return false;
    }

    strcpy(path_buf, path);
    rc = fsFsCreateDirectory(&fs, path_buf);
    fsFsClose(&fs);
    return R_SUCCEEDED(rc);
}

struct ConfigEntry {
    ConfigEntry(const char* _section, const char* _key, bool default_value) :
        section{_section}, key{_key}, value{default_value} {
            this->load_value_from_ini();
        }

    void load_value_from_ini() {
        this->value = ini_getbool(this->section, this->key, this->value, CONFIG_PATH);
    }

    auto create_list_item(const char* text) {
        auto item = new tsl::elm::ToggleListItem(text, value);
        item->setStateChangedListener([this](bool new_value){
            this->value = new_value;
            ini_putl(this->section, this->key, this->value, CONFIG_PATH);
        });
        return item;
    }

    const char* const section;
    const char* const key;
    bool value;
};

class GuiOptions final : public tsl::Gui {
public:
    GuiOptions() { }

    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("系统补丁", VERSION_WITH_HASH);
        auto list = new tsl::elm::List();

        list->addItem(new tsl::elm::CategoryHeader("选项"));
        list->addItem(config_patch_sysmmc.create_list_item("修补 真实系统"));
        list->addItem(config_patch_emummc.create_list_item("修补 虚拟系统"));
        list->addItem(config_logging.create_list_item("启用日志记录"));
        list->addItem(config_version_skip.create_list_item("跳过版本检测"));

        frame->setContent(list);
        return frame;
    }

    ConfigEntry config_patch_sysmmc{"options", "patch_sysmmc", true};
    ConfigEntry config_patch_emummc{"options", "patch_emummc", true};
    ConfigEntry config_logging{"options", "enable_logging", true};
    ConfigEntry config_version_skip{"options", "version_skip", true};
};

class GuiToggle final : public tsl::Gui {
public:
    GuiToggle() { }

    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("系统补丁", VERSION_WITH_HASH);
        auto list = new tsl::elm::List();

        list->addItem(new tsl::elm::CategoryHeader("文件系统(FS) - 0100000000000000"));
        list->addItem(config_noacidsigchk1.create_list_item("关闭ACID签名验证1"));
        list->addItem(config_noacidsigchk2.create_list_item("关闭ACID签名验证2"));
        list->addItem(config_noncasigchk_old.create_list_item("关闭旧版NCA签名验证"));
        list->addItem(config_noncasigchk_old2.create_list_item("关闭旧版NCA签名验证2"));
        list->addItem(config_noncasigchk_new.create_list_item("关闭新版NCA签名验证"));
        list->addItem(config_nocntchk.create_list_item("关闭CNT验证"));
        list->addItem(config_nocntchk2.create_list_item("关闭CNT验证2"));

        list->addItem(new tsl::elm::CategoryHeader("加载器(LDR) - 0100000000000001"));
        list->addItem(config_noacidsigchk.create_list_item("关闭ACID签名验证"));

        list->addItem(new tsl::elm::CategoryHeader("加密服务(ES) - 0100000000000033"));
        list->addItem(config_es1.create_list_item("加密服务(es1)"));
        list->addItem(config_es2.create_list_item("加密服务(es2)"));
        list->addItem(config_es3.create_list_item("加密服务(es3)"));
        list->addItem(config_es4.create_list_item("加密服务(es4)"));

        list->addItem(new tsl::elm::CategoryHeader("网络接口管理(NIFM) - 010000000000000F"));
        list->addItem(config_ctest.create_list_item("连接测试(ctest)"));
        list->addItem(config_ctest2.create_list_item("连接测试(ctest2)"));
        list->addItem(config_ctest3.create_list_item("连接测试(ctest3)"));

        list->addItem(new tsl::elm::CategoryHeader("网络安装管理(NIM) - 0100000000000025"));
        list->addItem(config_nim_old.create_list_item("旧版网络安装管理(nim)"));
        list->addItem(config_nim_new.create_list_item("新版网络安装管理(nim)"));

        list->addItem(new tsl::elm::CategoryHeader("禁用CA验证-全部应用"));
        list->addItem(config_ssl1.create_list_item("禁用CA验证1"));
        list->addItem(config_ssl2.create_list_item("禁用CA验证2"));
        list->addItem(config_ssl3.create_list_item("禁用CA验证3"));

        frame->setContent(list);
        return frame;
    }

    ConfigEntry config_noacidsigchk1{"fs", "noacidsigchk1", true};
    ConfigEntry config_noacidsigchk2{"fs", "noacidsigchk2", true};
    ConfigEntry config_noncasigchk_old{"fs", "noncasigchk_old", true};
    ConfigEntry config_noncasigchk_old2{"fs", "noncasigchk_old2", true};
    ConfigEntry config_noncasigchk_new{"fs", "noncasigchk_new", true};
    ConfigEntry config_nocntchk{"fs", "nocntchk", true};
    ConfigEntry config_nocntchk2{"fs", "nocntchk2", true};
    ConfigEntry config_noacidsigchk{"ldr", "noacidsigchk", true};
    ConfigEntry config_es1{"es", "es1", true};
    ConfigEntry config_es2{"es", "es2", true};
    ConfigEntry config_es3{"es", "es3", true};
    ConfigEntry config_es4{"es", "es4", true};
    ConfigEntry config_ctest{"nifm", "ctest", true};
    ConfigEntry config_ctest2{"nifm", "ctest2", true};
    ConfigEntry config_ctest3{"nifm", "ctest3", true};
    ConfigEntry config_nim_old{"nim_old", "nim_old", true};
    ConfigEntry config_nim_new{"nim_new", "nim_new", true};
    ConfigEntry config_ssl1{"ssl", "disablecaverification1", false};
    ConfigEntry config_ssl2{"ssl", "disablecaverification2", false};
    ConfigEntry config_ssl3{"ssl", "disablecaverification3", false};
};

class GuiLog final : public tsl::Gui {
public:
    GuiLog() { }

    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("系统补丁", VERSION_WITH_HASH);
        auto list = new tsl::elm::List();

        if (does_file_exist(LOG_PATH)) {
            struct CallbackUser {
                tsl::elm::List* list;
                std::string last_section;
            } callback_userdata{list};

            ini_browse([](const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, void *UserData){
                auto user = (CallbackUser*)UserData;
                std::string_view value{Value};

                if (value == "已跳过") {
                    return 1;
                }

                if (user->last_section != Section) {
                    user->last_section = Section;
                    user->list->addItem(new tsl::elm::CategoryHeader("日志：" + user->last_section));
                }

                #define F(x) ((x) >> 4) // 8bit -> 4bit
                constexpr tsl::Color colour_syspatch{F(0), F(255), F(200), F(255)};
                constexpr tsl::Color colour_file{F(255), F(177), F(66), F(255)};
                constexpr tsl::Color colour_unpatched{F(250), F(90), F(58), F(255)};
                #undef F

                if (value.starts_with("已修补")) {
                    if (value.ends_with("(sys-patch)")) {
                        user->list->addItem(new tsl::elm::ListItem(Key, "已修补", colour_syspatch));
                    } else {
                        user->list->addItem(new tsl::elm::ListItem(Key, "已修补", colour_file));
                    }
                } else if (value.starts_with("未修补") || value.starts_with("已禁用")) {
                    user->list->addItem(new tsl::elm::ListItem(Key, Value, colour_unpatched));
                } else if (user->last_section == "stats") {
                    user->list->addItem(new tsl::elm::ListItem(Key, Value, tsl::style::color::ColorDescription));
                } else {
                    user->list->addItem(new tsl::elm::ListItem(Key, Value, tsl::style::color::ColorText));
                }

                return 1;
            }, &callback_userdata, LOG_PATH);
        } else {
            list->addItem(new tsl::elm::ListItem("未找到日志！"));
        }

        frame->setContent(list);
        return frame;
    }
};

class GuiMain final : public tsl::Gui {
public:
    GuiMain() { }

    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("系统补丁", VERSION_WITH_HASH);
        auto list = new tsl::elm::List();

        auto options = new tsl::elm::ListItem("设置选项");
        auto toggle = new tsl::elm::ListItem("切换补丁状态");
        auto log = new tsl::elm::ListItem("查看日志");

        options->setClickListener([](u64 keys) -> bool {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiOptions>();
                return true;
            }
            return false;
        });

        toggle->setClickListener([](u64 keys) -> bool {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiToggle>();
                return true;
            }
            return false;
        });

        log->setClickListener([](u64 keys) -> bool {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiLog>();
                return true;
            }
            return false;
        });

        list->addItem(new tsl::elm::CategoryHeader("菜单"));
        list->addItem(options);
        list->addItem(toggle);
        list->addItem(log);

        frame->setContent(list);
        return frame;
    }
};

// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
class SysPatchOverlay final : public tsl::Overlay {
public:
    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiMain>();
    }
};

} // namespace

int main(int argc, char **argv) {
    create_dir("/config/");
    create_dir("/config/sys-patch/");
    return tsl::loop<SysPatchOverlay>(argc, argv);
}
