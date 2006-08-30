;;
;; Windows Gaim NSIS installer language macros
;;

!macro GAIM_MACRO_DEFAULT_STRING LABEL VALUE
  !ifndef "${LABEL}"
    !define "${LABEL}" "${VALUE}"
    !ifdef INSERT_DEFAULT
      !warning "${LANG} lang file missing ${LABEL}, using default..."
    !endif
  !endif
!macroend

!macro GAIM_MACRO_LANGSTRING_INSERT LABEL LANG
  LangString "${LABEL}" "${LANG_${LANG}}" "${${LABEL}}"
  !undef "${LABEL}"
!macroend

!macro GAIM_MACRO_LANGUAGEFILE_BEGIN LANG
  !define CUR_LANG "${LANG}"
!macroend

!macro GAIM_MACRO_LANGUAGEFILE_END
  !define INSERT_DEFAULT
  !include "${GAIM_DEFAULT_LANGFILE}"
  !undef INSERT_DEFAULT

  ; GAIM Language file Version 3
  ; String labels should match those from the default language file.

  ; Startup checks
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT INSTALLER_IS_RUNNING		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_IS_RUNNING			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_INSTALLER_NEEDED		${CUR_LANG}

  ; License Page
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_LICENSE_BUTTON			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_LICENSE_BOTTOM_TEXT		${CUR_LANG}

  ; Components Page
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SECTION_TITLE			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_SECTION_TITLE			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_THEMES_SECTION_TITLE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_NOTHEME_SECTION_TITLE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_WIMP_SECTION_TITLE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_BLUECURVE_SECTION_TITLE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_LIGHTHOUSEBLUE_SECTION_TITLE	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SHORTCUTS_SECTION_TITLE	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_DESKTOP_SHORTCUT_SECTION_TITLE ${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_STARTMENU_SHORTCUT_SECTION_TITLE	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SECTION_DESCRIPTION		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_SECTION_DESCRIPTION		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_THEMES_SECTION_DESCRIPTION	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_NO_THEME_DESC			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_WIMP_THEME_DESC			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_BLUECURVE_THEME_DESC		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_LIGHTHOUSEBLUE_THEME_DESC	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SHORTCUTS_SECTION_DESCRIPTION	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_DESKTOP_SHORTCUT_DESC		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_STARTMENU_SHORTCUT_DESC	${CUR_LANG}

  ; GTK+ Directory Page
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_UPGRADE_PROMPT			${CUR_LANG}

  ; Installer Finish Page
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_FINISH_VISIT_WEB_SITE		${CUR_LANG}

  ; Gaim Section Prompts and Texts
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_UNINSTALL_DESC			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_PROMPT_CONTINUE_WITHOUT_UNINSTALL	${CUR_LANG}

  ; GTK+ Section Prompts
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_INSTALL_ERROR			${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_BAD_INSTALL_PATH		${CUR_LANG}

  ; GTK+ Themes section
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GTK_NO_THEME_INSTALL_RIGHTS		${CUR_LANG}

  ; Uninstall Section Prompts
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT un.GAIM_UNINSTALL_ERROR_1		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT un.GAIM_UNINSTALL_ERROR_2		${CUR_LANG}

  ; Spellcheck Section Prompts
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_SECTION_TITLE	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_ERROR		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_DICT_ERROR		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_SECTION_DESCRIPTION	${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT ASPELL_INSTALL_FAILED		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_BRETON		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_CATALAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_CZECH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_WELSH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_DANISH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_GERMAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_ENGLISH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_GREEK		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_ESPERANTO		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_SPANISH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_FAROESE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_FRENCH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_ITALIAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_DUTCH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_NORWEGIAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_POLISH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_PORTUGUESE		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_ROMANIAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_RUSSIAN		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_SLOVAK		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_SWEDISH		${CUR_LANG}
  !insertmacro GAIM_MACRO_LANGSTRING_INSERT GAIM_SPELLCHECK_UKRAINIAN		${CUR_LANG}

  !undef CUR_LANG
!macroend

!macro GAIM_MACRO_INCLUDE_LANGFILE LANG FILE
  !insertmacro GAIM_MACRO_LANGUAGEFILE_BEGIN "${LANG}"
  !include "${FILE}"
  !insertmacro GAIM_MACRO_LANGUAGEFILE_END
!macroend
