/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#ifndef BLOODHORN_LOCALIZATION_H
#define BLOODHORN_LOCALIZATION_H

const wchar_t* GetLocalizedString(const char* key);
void SetLanguage(const char* lang_code);

#endif // BLOODHORN_LOCALIZATION_H 