{ This sub-script defines any custom wizard pages }
[Code]
Var
  SafetyPage: TWizardPage;
  SafetyURL: TLabel;
  SafetyTextBox: TLabel;

Procedure SafetyLinkOnClick(Sender: TObject);
Var
  ErrorCode: Integer;
Begin
  ShellExec('open', ExpandConstant('{cm:page_safetywarning_destination}'), '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
End;

Procedure InitializeWizard();
Begin
  SafetyPage := CreateCustomPage
  ( wpInstalling,
    ExpandConstant('{cm:page_safetywarning_title}'),
    ExpandConstant('{cm:page_safetywarning_subtitle}')
  );

  SafetyTextBox := TLabel.Create(SafetyPage);
  SafetyTextBox.Parent := SafetyPage.Surface;
  SafetyTextBox.Top := ScaleY(SafetyTextBox.Font.Size);
  SafetyTextBox.Caption := ExpandConstant('{cm:page_safetywarning_text}');
  SafetyTextBox.WordWrap := True;
  SafetyTextBox.Width := SafetyPage.SurfaceWidth;
  SafetyTextBox.Font.Size:= 10;

  SafetyURL := TLabel.Create(SafetyPage)
  SafetyURL.Parent := SafetyPage.Surface;
  SafetyURL.Top := SafetyTextBox.Height + SafetyTextBox.Top + ScaleY(SafetyTextBox.Font.Size);
  SafetyURL.Caption := ExpandConstant('{cm:page_safetywarning_link}');
  SafetyURL.Width := SafetyPage.SurfaceWidth;
  SafetyURL.Font.Size := SafetyTextBox.Font.Size;
  SafetyURL.Cursor := crHand;
  SafetyURL.OnClick := @SafetyLinkOnClick;
  { Alter Font *after* setting Parent so the correct defaults are inherited first }
  SafetyURL.Font.Style := SafetyURL.Font.Style + [fsUnderline];
  SafetyURL.Font.Color := clBlue;

  WizardForm.LicenseAcceptedRadio.Checked := true;
End;
