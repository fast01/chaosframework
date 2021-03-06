package it.infn.chaos.mds;

import com.vaadin.annotations.AutoGenerated;
import com.vaadin.ui.Alignment;
import com.vaadin.ui.Button;
import com.vaadin.ui.CustomComponent;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Label;
import com.vaadin.ui.TextArea;
import com.vaadin.ui.TextField;
import com.vaadin.ui.VerticalLayout;

public class USEditInfoVaadin extends CustomComponent {

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	@AutoGenerated
	private VerticalLayout		mainLayout;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_4;
	@AutoGenerated
	private Button				buttonCancell;
	@AutoGenerated
	private Button				buttonSaveInfo;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_1;
	@AutoGenerated
	private VerticalLayout		verticalLayout_2;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_2;
	@AutoGenerated
	private Button				buttonRemoveSecurityKeys;
	@AutoGenerated
	private Button				buttonGenerateSecurityKeys;
	@AutoGenerated
	private TextArea			textAreaPublicKey;
	@AutoGenerated
	private HorizontalLayout	horizontalLayoutUS;
	@AutoGenerated
	private TextField			textFieldUnitServerAlias;
	@AutoGenerated
	private Label				usLabel;

	/**
	 * The constructor should first build the main layout, set the composition root and then do any custom initialization.
	 * 
	 * The constructor will not be automatically regenerated by the visual editor.
	 */
	public USEditInfoVaadin() {
		buildMainLayout();
		setCompositionRoot(mainLayout);

		// TODO add user code here
	}

	public Button getButtonCancell() {
		return buttonCancell;
	}

	public Button getButtonSaveInfo() {
		return buttonSaveInfo;
	}

	public TextField getTextFieldUnitServerAlias() {
		return textFieldUnitServerAlias;
	}
	public Button getButtonGenerateSecurityKeys() {
		return buttonGenerateSecurityKeys;
	}

	public TextArea getTextAreaPublicKey() {
		return textAreaPublicKey;
	}
	public Button getButtonRemoveSecurityKeys() {
		return buttonRemoveSecurityKeys;
	}


	@AutoGenerated
	private VerticalLayout buildMainLayout() {
		// common part: create layout
		mainLayout = new VerticalLayout();
		mainLayout.setImmediate(false);
		mainLayout.setWidth("640px");
		mainLayout.setHeight("-1px");
		mainLayout.setMargin(false);

		// top-level component properties
		setWidth("640px");
		setHeight("-1px");

		// horizontalLayoutUS
		horizontalLayoutUS = buildHorizontalLayoutUS();
		mainLayout.addComponent(horizontalLayoutUS);

		// horizontalLayout_1
		horizontalLayout_1 = buildHorizontalLayout_1();
		mainLayout.addComponent(horizontalLayout_1);

		// horizontalLayout_4
		horizontalLayout_4 = buildHorizontalLayout_4();
		mainLayout.addComponent(horizontalLayout_4);

		return mainLayout;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayoutUS() {
		// common part: create layout
		horizontalLayoutUS = new HorizontalLayout();
		horizontalLayoutUS.setImmediate(false);
		horizontalLayoutUS.setWidth("100.0%");
		horizontalLayoutUS.setHeight("-1px");
		horizontalLayoutUS.setMargin(true);
		horizontalLayoutUS.setSpacing(true);

		// usLabel
		usLabel = new Label();
		usLabel.setImmediate(false);
		usLabel.setWidth("-1px");
		usLabel.setHeight("-1px");
		usLabel.setValue("Unit Server:");
		horizontalLayoutUS.addComponent(usLabel);

		// textFieldUnitServerAlias
		textFieldUnitServerAlias = new TextField();
		textFieldUnitServerAlias.setImmediate(false);
		textFieldUnitServerAlias.setWidth("100.0%");
		textFieldUnitServerAlias.setHeight("-1px");
		horizontalLayoutUS.addComponent(textFieldUnitServerAlias);
		horizontalLayoutUS.setExpandRatio(textFieldUnitServerAlias, 1.0f);

		return horizontalLayoutUS;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_1() {
		// common part: create layout
		horizontalLayout_1 = new HorizontalLayout();
		horizontalLayout_1.setImmediate(false);
		horizontalLayout_1.setWidth("100.0%");
		horizontalLayout_1.setHeight("-1px");
		horizontalLayout_1.setMargin(false);

		// verticalLayout_2
		verticalLayout_2 = buildVerticalLayout_2();
		horizontalLayout_1.addComponent(verticalLayout_2);

		return horizontalLayout_1;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_2() {
		// common part: create layout
		verticalLayout_2 = new VerticalLayout();
		verticalLayout_2.setImmediate(false);
		verticalLayout_2.setWidth("100.0%");
		verticalLayout_2.setHeight("-1px");
		verticalLayout_2.setMargin(false);
		verticalLayout_2.setSpacing(true);

		// textAreaPublicKey
		textAreaPublicKey = new TextArea();
		textAreaPublicKey.setCaption("Public Key:");
		textAreaPublicKey.setImmediate(false);
		textAreaPublicKey.setWidth("100.0%");
		textAreaPublicKey.setHeight("-1px");
		verticalLayout_2.addComponent(textAreaPublicKey);

		// horizontalLayout_2
		horizontalLayout_2 = buildHorizontalLayout_2();
		verticalLayout_2.addComponent(horizontalLayout_2);

		return verticalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_2() {
		// common part: create layout
		horizontalLayout_2 = new HorizontalLayout();
		horizontalLayout_2.setImmediate(false);
		horizontalLayout_2.setWidth("-1px");
		horizontalLayout_2.setHeight("-1px");
		horizontalLayout_2.setMargin(false);

		// buttonGenerateSecurityKeys
		buttonGenerateSecurityKeys = new Button();
		buttonGenerateSecurityKeys.setCaption("Generate Securty Keys");
		buttonGenerateSecurityKeys.setImmediate(true);
		buttonGenerateSecurityKeys.setWidth("-1px");
		buttonGenerateSecurityKeys.setHeight("-1px");
		horizontalLayout_2.addComponent(buttonGenerateSecurityKeys);

		// buttonRemoveSecurityKeys
		buttonRemoveSecurityKeys = new Button();
		buttonRemoveSecurityKeys.setCaption("Remove Security Keys");
		buttonRemoveSecurityKeys.setImmediate(false);
		buttonRemoveSecurityKeys.setWidth("-1px");
		buttonRemoveSecurityKeys.setHeight("-1px");
		horizontalLayout_2.addComponent(buttonRemoveSecurityKeys);

		return horizontalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_4() {
		// common part: create layout
		horizontalLayout_4 = new HorizontalLayout();
		horizontalLayout_4.setImmediate(false);
		horizontalLayout_4.setWidth("-1px");
		horizontalLayout_4.setHeight("-1px");
		horizontalLayout_4.setMargin(false);
		horizontalLayout_4.setSpacing(true);

		// buttonSaveInfo
		buttonSaveInfo = new Button();
		buttonSaveInfo.setCaption("Save");
		buttonSaveInfo.setImmediate(true);
		buttonSaveInfo.setWidth("-1px");
		buttonSaveInfo.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonSaveInfo);

		// buttonCancell
		buttonCancell = new Button();
		buttonCancell.setCaption("Cancell");
		buttonCancell.setImmediate(true);
		buttonCancell.setWidth("-1px");
		buttonCancell.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonCancell);
		horizontalLayout_4.setComponentAlignment(buttonCancell, new Alignment(33));

		return horizontalLayout_4;
	}

}
