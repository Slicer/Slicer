; Copyright Safing ICS Technologies GmbH. Use of this source code is governed by the Apache 2 license that can be found in the LICENSE file.
; Adapted from https://github.com/safing/nsis-shortcut-properties/blob/b5ede73fc49665449731df5d0f945192329b0ef6/shortcut-properties.nsh

!include LogicLib.nsh
!include Win\Propkey.nsh
!include Win\COM.nsh

!define /ifndef PKEY_AppUserModel_ToastActivatorCLSID '"{9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3}",26'
; The GUID of the property key, not the set CLSID DO NOT CHANGE!
!define /ifndef VT_CLSID 72
!define /ifndef STGM_READWRITE 2

; Errorhandler
; if $0 != 0, break and output Returncode and description of the current point in code; r0 is always used for the returncodes of the System::Call
; TODO: safely clean Memory on Error
!define Shortcut_returnOnError `!insertmacro Shortcut_returnOnError $0`
!macro Shortcut_returnOnError _retVal _Msg
${If} ${_retVal} <> 0 ; (<>) == (!=)
	DetailPrint "Error-Point: ${_Msg}"
	DetailPrint "Error-Code: ${_retVal}"
	Goto Shortcut_onError
${EndIf}
!macroend

;;;
; Sets specified AppID and CLSID as File-Property in the way needed for Toast Notifications
; Pushes return-value (Windows HRESULT) to stack (0: Success)
; Example: !insertmacro ShortcutSetAppID "$SMPROGRAMS\test.lnk" "{32860D72-BA7F-64CC-AF50-72B6FB1ECE26}" "com.example.nsisdemo" ; DON'T USE THIS GUID!!!
; WARNING: Because of the limited lifetime of the installer and the minimal RAM usage, it wasn't taken care of freeing all allocated stuff, especially
; in the case, something goes wrong
;;;
!macro ShortcutSetAppID ShortcutPath CLSID AppID

DetailPrint "Add Toast-Properties to Shortcut"

System::Store S ; Seems to save the variables $0-$9 somewhere so we can use this variables without danger


; internal Variable-Usage:
; $0: Return-Value of operations (was successful?)
; $1: COM-Object-Pointer
; $2: IPersistFile-Pointer
; $3: IPropertyStore-Pointer
; $4: Propertystore-Key-Struct
; $5: Propertystore-Val-Struct
; $6: temporary Property-Val extra storage (if val is pointer)
; $7: temporary length OR strlen of a string

; Set $0-$9 to known invalid values as additional prevention of unwanted behaviour
IntOp $0 0 - 1
IntOp $1 0 + 0
IntOp $2 0 + 0
IntOp $3 0 + 0
IntOp $4 0 + 0
IntOp $5 0 + 0
IntOp $6 0 + 0
IntOp $7 0 - 1
IntOp $8 0 + 0
IntOp $9 0 + 0

!insertmacro ComHlpr_CreateInProcInstance ${CLSID_ShellLink} ${IID_IShellLink} r1 ".r0" ; creates ComHlpr_CreateInProcInstance into $1
${Shortcut_returnOnError} "CoCreateInstance" ; check for Errors

${IUnknown::QueryInterface} $1 '("${IID_IPersistFile}",.r2)i.r0' ; creates IPersistFile into $2
${Shortcut_returnOnError} "Create IPersistFile QueryInterface"

${IPersistFile::Load} $2 "('${ShortcutPath}', ${STGM_READWRITE})i.r0" ; Loads Shortcut
${Shortcut_returnOnError} "Load Shortcut"

${IUnknown::QueryInterface} $1 '("${IID_IPropertyStore}",.r3)i.r0' ; creates IPropertyStore into $3
${Shortcut_returnOnError} "Create IID_IPropertyStore QueryInterface"


;;;
; CLSID
;;;
System::Call '*${SYSSTRUCT_PROPERTYKEY}(${PKEY_AppUserModel_ToastActivatorCLSID})p.r4' ; Sets up Struct for PropertyKey (CLSID)
System::Call '*(&g16 "${CLSID}")p.r6' ; Store CLSID into struct just to have the Value stored somewhere we can point to
System::Call '*${SYSSTRUCT_PROPVARIANT}(${VT_CLSID},,p r6)p.r5'  ; Sets up Struct for PropertyValue (actually, the value is a pointer to the actual value)

System::Free $6 ; Free temporary value-struct
; The structs $4 and $5 are reused for AppID


;;;
; AppID
;;;
; $4 and $5 are reused, $6 is reallocated

System::Call '*$4${SYSSTRUCT_PROPERTYKEY}(${PKEY_AppUserModel_ID})' ; Loads PropertyKey into already existing struct $4

; Calculate the Bytes needed to allocate for copying the AppID as wstring, write number into $7
StrLen $7 "${AppID}"
IntOp $7 $7 + 1 ; incl. \0
IntOp $7 $7 * 2 ; wchars are 2 Byte wide in Windows
System::Call "ole32::CoTaskMemAlloc(i $7)p.r6"
; allocate $7 Bytes into $6 for temporarily storing the AppID as WSTRING (a pointer to the String is required, so we need manual allocation)

${If} $6 = 0 ; Is the pointer to the allocated Memory NULL? (= No space left for allocation, then return a Error)
	DetailPrint "No RAM could be allocated for AppID"
	Goto Shortcut_onError
${EndIf}

StrLen $7 ${AppID} ; Now store the stringlength of the AppID into $7, not the Bytecount!
IntOp $7 $7 + 1 ; Add \0
System::Call '*$6(&w$7 "${AppID}")' ; memcpy (wstring)AppID into $6 the temporary storage of the PropertyValue
System::Call '*$5${SYSSTRUCT_PROPVARIANT}(${VT_LPWSTR},,p r6)' ; Load Property-Val into $5

${IPropertyStore::SetValue} $3 '($4,$5)i.r0' ; Actually sets Key-Value-Pair in IPropertyStore-Object ($3)
${Shortcut_returnOnError} "Save AppID"

${IPropertyStore::Commit} $3 "i.r0" ; Commit changes in IPropertyStore-Object ($3)
${Shortcut_returnOnError} "Commit changes to PropertyStore"

${IPersistFile::Save} $2 '("${ShortcutPath}",1)r.r0' ; Save changes into Shortcut at ${ShortcutPath} (IPersistFile at $2)
${Shortcut_returnOnError} "Save Shortcut"

; Freeing&Releasing stuff (TODO: do in case of Error?)
System::Call "ole32::CoTaskMemFree(p r6)"
; Free the temporary Storage; It was allocated using ole32::CoTaskMemAlloc(),
; therefore it needs to be freeed using ole32::CoTaskMemFree()
System::Free $4 ; Freeing PropertyKey-Struct
System::Free $5 ; Freeing PropertyValue-Struct
${IUnknown::Release} $3 "" ; Releasing IPropertyStore-Object at $3
${IUnknown::Release} $2 "" ; Releasing IPersistFile at $2
${IUnknown::Release} $1 "" ; Releasing COM-Object at $1

DetailPrint "Successfully added Toast-Properties to Shortcut"

Shortcut_onError: ; Label to which is jumped on previous Error
push $0 ; pushes return Value to Stack
System::Store L ; Seems to load the previous values of $0-$9 ...
!macroend
