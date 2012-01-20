package it.infn.chaos.mds;

import com.vaadin.annotations.AutoGenerated;
import com.vaadin.ui.Alignment;
import com.vaadin.ui.Button;
import com.vaadin.ui.CustomComponent;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Table;
import com.vaadin.ui.VerticalLayout;

public class LiveDataPreferenceVaadin extends CustomComponent {

	@AutoGenerated
	private VerticalLayout	mainLayout;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_4;
	@AutoGenerated
	private Button	buttonInsert;
	@AutoGenerated
	private Button	buttonDelete;
	@AutoGenerated
	private Button	buttonClose;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_3;
	@AutoGenerated
	private Table	table_1;

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	
	

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	/**
	 * 
	 */
	private static final long	serialVersionUID	= 2391845585290163693L;

	/**
	 * The constructor should first build the main layout, set the composition root and then do any
	 * custom initialization.
	 * 
	 * The constructor will not be automatically regenerated by the visual editor.
	 */
	public LiveDataPreferenceVaadin() {
		buildMainLayout();
		setCompositionRoot(mainLayout);

		// TODO add user code here
	}

	public Button getButtonClose() {
		return buttonClose;
	}
	public Button getButtonInsert() {
		return buttonInsert;
	}
	public Button getButtonDelete() {
		return buttonDelete;
	}
	public Table getPreferenceTable(){
		return table_1;
	}

	@AutoGenerated
	private VerticalLayout buildMainLayout() {
		// common part: create layout
		mainLayout = new VerticalLayout();
		mainLayout.setImmediate(false);
		mainLayout.setWidth("100%");
		mainLayout.setHeight("100%");
		mainLayout.setMargin(false);
		
		// top-level component properties
		setWidth("100.0%");
		setHeight("100.0%");
		
		// horizontalLayout_3
		horizontalLayout_3 = buildHorizontalLayout_3();
		mainLayout.addComponent(horizontalLayout_3);
		mainLayout.setExpandRatio(horizontalLayout_3, 1.0f);
		mainLayout.setComponentAlignment(horizontalLayout_3, new Alignment(48));
		
		// horizontalLayout_4
		horizontalLayout_4 = buildHorizontalLayout_4();
		mainLayout.addComponent(horizontalLayout_4);
		mainLayout.setComponentAlignment(horizontalLayout_4, new Alignment(48));
		
		return mainLayout;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_3() {
		// common part: create layout
		horizontalLayout_3 = new HorizontalLayout();
		horizontalLayout_3.setImmediate(false);
		horizontalLayout_3.setWidth("100.0%");
		horizontalLayout_3.setHeight("100.0%");
		horizontalLayout_3.setMargin(false);
		
		// table_1
		table_1 = new Table();
		table_1.setImmediate(false);
		table_1.setWidth("100.0%");
		table_1.setHeight("100.0%");
		horizontalLayout_3.addComponent(table_1);
		horizontalLayout_3.setComponentAlignment(table_1, new Alignment(48));
		
		return horizontalLayout_3;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_4() {
		// common part: create layout
		horizontalLayout_4 = new HorizontalLayout();
		horizontalLayout_4.setImmediate(false);
		horizontalLayout_4.setWidth("-1px");
		horizontalLayout_4.setHeight("-1px");
		horizontalLayout_4.setMargin(true);
		horizontalLayout_4.setSpacing(true);
		
		// buttonClose
		buttonClose = new Button();
		buttonClose.setCaption("Close");
		buttonClose.setImmediate(true);
		buttonClose.setWidth("-1px");
		buttonClose.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonClose);
		horizontalLayout_4.setComponentAlignment(buttonClose, new Alignment(34));
		
		// buttonDelete
		buttonDelete = new Button();
		buttonDelete.setCaption("Delete Server");
		buttonDelete.setImmediate(false);
		buttonDelete.setWidth("-1px");
		buttonDelete.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonDelete);
		horizontalLayout_4.setComponentAlignment(buttonDelete, new Alignment(34));
		
		// buttonInsert
		buttonInsert = new Button();
		buttonInsert.setCaption("New Server");
		buttonInsert.setImmediate(true);
		buttonInsert.setWidth("-1px");
		buttonInsert.setHeight("-1px");
		horizontalLayout_4.addComponent(buttonInsert);
		horizontalLayout_4.setComponentAlignment(buttonInsert, new Alignment(34));
		
		return horizontalLayout_4;
	}
}
